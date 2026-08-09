/*
 * ==============================================================================
 * TINY ARC BUDDY - AFFINE LINEAR FUNCTION EXPLORER (Integer & Live Equation Edition)
 * ==============================================================================
 */

#include <avr/io.h>
#include <util/delay.h>

#include "font.h"     
#include "graphics.h" 
#include "strings.h"

#define OWNER_NAME "ANNA"           
#define BUILD_DATE "25.04.2026"      

#define OLED_SDA PB3  
#define OLED_SCL PB4  
#define BTN_LEFT PB0  
#define BTN_RIGHT PB2 
#define BUZZER PB1    

byte appState = 0; 
bool needsRedraw = true; 

// --- App Settings ---
int selectedMode = 0;  // 0 = Alg->Geom, 1 = Geom->Alg
int functionCount = 1; // 1 or 2 lines

// --- Data Storage ---
// Algebra Mode: [k1, d1, k2, d2]
// Geometry Mode:[p1x, p1y, p2x, p2y, p3x, p3y, p4x, p4y]
int dataVals[8];     
int currentInput = 0; 
int inputStep = 0;    

unsigned long lastMoveTime = 0;
bool leftWasPressed = false;
bool rightWasPressed = false;
bool fireWasPressed = false;

// ------------------------------------------------------------------------------
// I2C BIT-BANGING & GRAPHICS ENGINE
// ------------------------------------------------------------------------------
inline void i2c_start() {
  PORTB |= (1 << OLED_SDA) | (1 << OLED_SCL);
  PORTB &= ~(1 << OLED_SDA); PORTB &= ~(1 << OLED_SCL);
}
inline void i2c_stop() {
  PORTB &= ~(1 << OLED_SDA); PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
}
void i2c_write(uint8_t data) {
  for (uint8_t i = 0; i < 8; i++) {
    if (data & 0x80) PORTB |= (1 << OLED_SDA); else PORTB &= ~(1 << OLED_SDA);
    PORTB |= (1 << OLED_SCL); PORTB &= ~(1 << OLED_SCL);
    data <<= 1;
  }
  PORTB |= (1 << OLED_SCL); PORTB &= ~(1 << OLED_SCL);
}
void oled_command(uint8_t cmd) {
  i2c_start(); i2c_write(0x78); i2c_write(0x00); i2c_write(cmd); i2c_stop();
}
void oled_set_cursor(uint8_t x, uint8_t y) {
  oled_command(0xB0 + y); oled_command(0x00 + (x & 0x0F)); oled_command(0x10 + ((x >> 4) & 0x0F)); 
}
void oled_clear() {
  for (uint8_t y = 0; y < 8; y++) {
    oled_set_cursor(0, y);
    i2c_start(); i2c_write(0x78); i2c_write(0x40);
    for (uint8_t x = 0; x < 128; x++) i2c_write(0x00);
    i2c_stop();
  }
}

void drawChar(uint8_t x, uint8_t page, char c) {
  uint8_t idx = 0; bool isSpace = false;
  if (c >= '0' && c <= '9') idx = c - '0' + 1;
  else if (c >= 'a' && c <= 'z') idx = (c - 32) - 'A' + 11; 
  else if (c >= 'A' && c <= 'Z') idx = c - 'A' + 11;
  else if (c == '\'') idx = 37; else if (c == '@') idx = 38;
  else if (c == ':') idx = 39; else if (c == '.') idx = 40;
  else if (c == '-') idx = 41; else if (c == '=') idx = 42;
  else if (c == '(' || c == '[') idx = 43; else if (c == ')' || c == ']') idx = 44;
  else if (c == '^') idx = 45; else if (c == '%') idx = 46;
  else if (c == '+') idx = 47; else if (c == '*') idx = 48;
  else if (c == '/') idx = 49; else if (c == ',') idx = 50;
  else if (c == '_') idx = 51; else if (c == ' ') { isSpace = true; }
  
  oled_set_cursor(x, page);
  i2c_start(); i2c_write(0x78); i2c_write(0x40);
  
  if (isSpace || idx == 0) for(uint8_t i=0; i<6; i++) i2c_write(0x00);
  else { for(uint8_t i=0; i<5; i++) i2c_write(pgm_read_byte(&font5x7[idx * 5 + i])); i2c_write(0x00); }
  i2c_stop();
}

void drawString(uint8_t x, uint8_t page, const char* str) {
  while (*str) { drawChar(x, page, *str++); x += 6; }
}
void drawStringPGM(uint8_t x, uint8_t page, const char* pgmStr) {
  char c; while ((c = pgm_read_byte(pgmStr++)) != 0) { drawChar(x, page, c); x += 6; }
}
void clearRestOfLine(uint8_t x, uint8_t page) {
  while (x < 124) { drawChar(x, page, ' '); x += 6; }
}
void drawInt(uint8_t x, uint8_t page, int val) {
  char str[8]; itoa(val, str, 10); drawString(x, page, str);
}

// Live Equation Element Renderer
void drawEqElem(uint8_t &x, uint8_t page, int val, bool isActive, char suffix) {
  if (isActive) { drawChar(x, page, '['); x+=6; }
  else { drawChar(x, page, ' '); x+=6; }

  char buf[8]; itoa(val, buf, 10);
  drawString(x, page, buf); x+= strlen(buf)*6;

  if (isActive) { drawChar(x, page, ']'); x+=6; }
  else { drawChar(x, page, ' '); x+=6; }

  if (suffix != ' ') { drawChar(x, page, suffix); x+=6; }
}

void printEqCompact(uint8_t &x, uint8_t page, int k, int d) {
  if (k == 999) {
    drawString(x, page, "x="); x+=12; drawInt(x, page, d); char b[5]; itoa(d,b,10); x+=strlen(b)*6;
  } else {
    drawString(x, page, "y="); x+=12;
    drawInt(x, page, k); char b1[5]; itoa(k,b1,10); x+=strlen(b1)*6;
    drawChar(x, page, 'x'); x+=6;
    if (d >= 0) { drawChar(x, page, '+'); x+=6; }
    drawInt(x, page, d); char b2[5]; itoa(d,b2,10); x+=strlen(b2)*6;
  }
}

void drawFixedPoint(uint8_t x, uint8_t page, long val) {
  if (val < 0) { drawChar(x, page, '-'); x += 6; val = -val; }
  long whole = val / 100; int frac = val % 100;
  if (frac < 0) frac = -frac;
  char buf[12]; itoa(whole, buf, 10); int len = strlen(buf);
  buf[len] = '.'; buf[len+1] = (frac / 10) + '0'; buf[len+2] = (frac % 10) + '0'; buf[len+3] = '\0';
  drawString(x, page, buf);
}
void drawLargeSprite(uint8_t x, uint8_t startPage, const uint8_t* sprite, uint8_t width, uint8_t pages) {
  for (uint8_t p = 0; p < pages; p++) {
    oled_set_cursor(x, startPage + p);
    i2c_start(); i2c_write(0x78); i2c_write(0x40);
    for(uint8_t i = 0; i < width; i++) i2c_write(pgm_read_byte(&sprite[(p * width) + i]));
    i2c_stop();
  }
}

void beep(int freq, int duration) {
  int halfPeriod = 1000000L / freq / 2;
  long loops = (long)duration * 1000L / (halfPeriod * 2);
  for (long i=0; i<loops; i++) {
    PORTB |= (1 << BUZZER); delayMicroseconds(halfPeriod);
    PORTB &= ~(1 << BUZZER); delayMicroseconds(halfPeriod);
  }
}

// ------------------------------------------------------------------------------
// REAL-TIME GRAPHING ENGINE (Strict Integer Math)
// ------------------------------------------------------------------------------
uint8_t getLineBits(int y1, int y2, int page) {
  int pTop = page * 8;
  int pBot = pTop + 7;
  int minY = (y1 < y2) ? y1 : y2;
  int maxY = (y1 > y2) ? y1 : y2;
  if (minY > pBot || maxY < pTop) return 0;
  
  uint8_t bits = 0;
  int start = (minY > pTop) ? minY : pTop;
  int end = (maxY < pBot) ? maxY : pBot;
  for (int y = start; y <= end; y++) bits |= (1 << (y - pTop));
  return bits;
}

// Center (64, 32). Scale: 1 unit = 2 pixels.
void renderCoordinateGraph(int k1, int d1, int k2, int d2, 
                           bool drawOne, bool drawTwo,
                           bool showCursor = false, int cursorX = 0, int cursorY = 0, bool lockX = false,
                           int* pts = nullptr, int numPts = 0) {
  
  for (uint8_t p = 1; p <= 6; p++) {
    oled_set_cursor(0, p);
    i2c_start(); i2c_write(0x78); i2c_write(0x40);

    for (int x = 0; x < 128; x++) {
      uint8_t data = 0;
      
      // Axes and Grid
      if (x == 64) data |= 0xFF; 
      else if (x % 10 == 4) data |= 0x11; 
      
      if (p == 4) {
        data |= 0x01; 
        if (x % 4 == 0) data |= 0x03; 
      }

      // Line 1
      if (drawOne && k1 != 999) {
        long y_px_1 = 32 - (k1 * (x - 64)) - (d1 * 2);
        long y_px_2 = 32 - (k1 * (x + 1 - 64)) - (d1 * 2);
        data |= getLineBits(y_px_1, y_px_2, p);
      } else if (drawOne && k1 == 999) {
        if (x == 64 + (d1 * 2)) data |= 0xFF; // Vertical line
      }

      // Line 2
      if (drawTwo && k2 != 999) {
        long y_px_1 = 32 - (k2 * (x - 64)) - (d2 * 2);
        long y_px_2 = 32 - (k2 * (x + 1 - 64)) - (d2 * 2);
        if (x % 2 == 0) data |= getLineBits(y_px_1, y_px_2, p); // Dashed
      } else if (drawTwo && k2 == 999) {
        if (x == 64 + (d2 * 2) && x % 2 == 0) data |= 0xFF; 
      }

      // Input Cursor & Points
      if (showCursor) {
        int scX = 64 + (cursorX * 2);
        int scY = 32 - (cursorY * 2);
        if (x == scX) { if (lockX) data |= 0xFF; else data |= 0x55; }
        if (lockX) {
          int pTop = p * 8; int pBot = pTop + 7;
          if (scY >= pTop && scY <= pBot) data |= 0xFF; 
        }
      }

      if (pts != nullptr) {
        for (int i=0; i<numPts; i+=2) {
          int px = 64 + pts[i]*2;
          int py = 32 - pts[i+1]*2;
          if (x == px) {
            int pTop = p * 8; int pBot = pTop + 7;
            if (py >= pTop && py <= pBot) data |= 0xF0; // Dot marker
          }
        }
      }

      i2c_write(data);
    }
    i2c_stop();
  }
}

// ------------------------------------------------------------------------------
// APPLICATION LOGIC
// ------------------------------------------------------------------------------

bool readLeft() { return (PINB & (1 << BTN_LEFT)); } 
bool readRight() { return (PINB & (1 << BTN_RIGHT)); } 
bool readFire() {
  ADCSRA |= (1 << ADSC); while (ADCSRA & (1 << ADSC)); return (ADC < 950); 
}

void setup() {
  DDRB |= (1 << OLED_SDA) | (1 << OLED_SCL) | (1 << BUZZER);
  DDRB &= ~((1 << BTN_LEFT) | (1 << BTN_RIGHT));
  PORTB &= ~((1 << BTN_LEFT) | (1 << BTN_RIGHT)); 
  ADMUX = 0b00000000; ADCSRA = 0b10000011; 

  oled_command(0xAE); oled_command(0x20); oled_command(0x02); 
  oled_command(0xA1); oled_command(0xC8); oled_command(0x8D); 
  oled_command(0x14); oled_command(0xAF); 
}

void loop() {
  unsigned long currentMillis = millis();

  // STATE 0: SPLASH
  if (appState == 0) {
    oled_clear(); drawLargeSprite(48, 2, htl_logo, 32, 4); delay(3000); 
    appState = 1; needsRedraw = true; oled_clear();
  }

  // STATE 1: CREDITS
  if (appState == 1) {
    if (needsRedraw) {
      drawString(3, 0, "GIRLS' DAY"); drawString(3, 1, "@ HTL KAINDORF");
      drawString(3, 3, "BUILT BY:"); drawString(60, 3, OWNER_NAME); 
      drawString(3, 4, "ON "); drawString(25, 4, BUILD_DATE);  
      drawString(2, 7, "> FIRE TO OPEN");
      needsRedraw = false;
    }
    if (readFire()) { beep(1000, 100); appState = 2; needsRedraw = true; oled_clear(); while(readFire()){} }
  }

  // STATE 2: MODE SELECT
  if (appState == 2) {
    if (needsRedraw) {
      drawString(10, 1, "MODE:");
      uint8_t x = 15;
      const char* mStr = (const char*)pgm_read_word(&modeStrings[selectedMode]);
      drawStringPGM(x, 4, mStr);
      x += strlen_P(mStr) * 6; clearRestOfLine(x, 4);
      needsRedraw = false;
    }
    
    if (readLeft() && !leftWasPressed) { selectedMode ^= 1; beep(1000, 20); needsRedraw = true; } 
    leftWasPressed = readLeft();

    if (readRight() && !rightWasPressed) { selectedMode ^= 1; beep(1000, 20); needsRedraw = true; } 
    rightWasPressed = readRight();

    if (readFire() && !fireWasPressed) {
      beep(1500, 50); appState = 3; needsRedraw = true; oled_clear(); delay(200);
    } fireWasPressed = readFire();
  }

  // STATE 3: COUNT SELECT
  if (appState == 3) {
    if (needsRedraw) {
      drawString(10, 1, "LINES:");
      uint8_t x = 15;
      const char* cStr = (const char*)pgm_read_word(&countStrings[functionCount - 1]);
      drawStringPGM(x, 4, cStr);
      x += strlen_P(cStr) * 6; clearRestOfLine(x, 4);
      needsRedraw = false;
    }
    
    if (readLeft() && !leftWasPressed) { functionCount = 3 - functionCount; beep(1000, 20); needsRedraw = true; } 
    leftWasPressed = readLeft();

    if (readRight() && !rightWasPressed) { functionCount = 3 - functionCount; beep(1000, 20); needsRedraw = true; } 
    rightWasPressed = readRight();

    if (readFire() && !fireWasPressed) {
      beep(2000, 50);
      for(uint8_t i=0; i<8; i++) dataVals[i] = 0;
      inputStep = 0; currentInput = 0; appState = 4; 
      needsRedraw = true; oled_clear(); delay(200);
    } fireWasPressed = readFire();
  }

  // STATE 4: INPUT ENGINE
  if (appState == 4) {
    
    // --- 4A: ALGEBRA INPUT (Live Formula) ---
    if (selectedMode == 0) {
      if (needsRedraw) {
        drawString(0, 0, "ALGEBRA INPUT:");
        uint8_t cx = 5;
        drawString(cx, 3, "f1: y="); cx += 36;
        drawEqElem(cx, 3, (inputStep==0 ? currentInput : dataVals[0]), inputStep==0, 'x');
        drawChar(cx, 3, '+'); cx += 6;
        drawEqElem(cx, 3, (inputStep==1 ? currentInput : dataVals[1]), inputStep==1, ' ');
        clearRestOfLine(cx, 3);

        if (functionCount == 2) {
          cx = 5;
          drawString(cx, 5, "f2: y="); cx += 36;
          drawEqElem(cx, 5, (inputStep==2 ? currentInput : dataVals[2]), inputStep==2, 'x');
          drawChar(cx, 5, '+'); cx += 6;
          drawEqElem(cx, 5, (inputStep==3 ? currentInput : dataVals[3]), inputStep==3, ' ');
          clearRestOfLine(cx, 5);
        }
        needsRedraw = false;
      }
      
      if (readLeft()) {
        if (!leftWasPressed) { currentInput--; lastMoveTime = currentMillis; beep(800, 5); needsRedraw = true; } 
        else if (currentMillis - lastMoveTime > 300) { currentInput -= 5; delay(40); beep(800, 5); needsRedraw = true; }
      } leftWasPressed = readLeft();
  
      if (readRight()) {
        if (!rightWasPressed) { currentInput++; lastMoveTime = currentMillis; beep(1200, 5); needsRedraw = true; } 
        else if (currentMillis - lastMoveTime > 300) { currentInput += 5; delay(40); beep(1200, 5); needsRedraw = true; }
      } rightWasPressed = readRight();
  
      if (readFire() && !fireWasPressed) {
        beep(2000, 50);
        dataVals[inputStep] = currentInput;
        inputStep++; currentInput = 0; needsRedraw = true; delay(200);
  
        int maxSteps = (functionCount == 1) ? 2 : 4;
        if (inputStep >= maxSteps) { appState = 5; oled_clear(); }
      } fireWasPressed = readFire();
    }
    
    // --- 4B: GEOMETRY INPUT (Coordinate Cross) ---
    else if (selectedMode == 1) {
      bool isMovingX = (inputStep % 2 == 0);
      int activeLine = (inputStep < 4) ? 1 : 2;
      int activePt = (inputStep % 4 < 2) ? 1 : 2;
      
      if (needsRedraw) {
        drawString(0, 0, "F"); drawInt(6, 0, activeLine);
        drawString(18, 0, "P"); drawInt(24, 0, activePt);
        
        uint8_t px = 40; drawString(px, 0, "X=[");
        if (isMovingX) {
          char buf[4]; itoa(currentInput, buf, 10); drawString(px+18, 0, buf); px += 18 + strlen(buf)*6;
          drawChar(px, 0, ']'); clearRestOfLine(px+6, 0);
        } else {
          char buf[4]; itoa(dataVals[inputStep-1], buf, 10); drawString(px+18, 0, buf); px += 18 + strlen(buf)*6;
          drawChar(px, 0, ']'); px+=6;
          drawString(px, 0, " Y=["); px+=24;
          char buf2[4]; itoa(currentInput, buf2, 10); drawString(px, 0, buf2); px += strlen(buf2)*6;
          drawChar(px, 0, ']'); clearRestOfLine(px+6, 0);
        }

        int cx = isMovingX ? currentInput : dataVals[inputStep-1];
        int cy = isMovingX ? 0 : currentInput;
        
        renderCoordinateGraph(0, 0, 0, 0, false, false, true, cx, cy, !isMovingX, dataVals, inputStep);
        needsRedraw = false;
      }
      
      if (readLeft() && !leftWasPressed) {
        if (currentInput > -10) currentInput--; beep(800, 10); needsRedraw = true;
      } leftWasPressed = readLeft();

      if (readRight() && !rightWasPressed) {
        if (currentInput < 10) currentInput++; beep(1200, 10); needsRedraw = true;
      } rightWasPressed = readRight();

      if (readFire() && !fireWasPressed) {
        beep(2000, 50);
        dataVals[inputStep] = currentInput;
        inputStep++; currentInput = 0; needsRedraw = true; delay(200);
        
        int maxSteps = (functionCount == 1) ? 4 : 8;
        if (inputStep >= maxSteps) { appState = 5; oled_clear(); }
      } fireWasPressed = readFire();
    }
  }

  // STATE 5: GRAPH PLOTTER & ALGEBRA RESULTS
  if (appState == 5) {
    if (needsRedraw) {
      
      int k1=0, d1=0, k2=0, d2=0;

      // --- MATHEMATICS ROUTINE ---
      if (selectedMode == 0) {
        k1 = dataVals[0]; d1 = dataVals[1];
        if (functionCount == 2) { k2 = dataVals[2]; d2 = dataVals[3]; }
      } 
      else if (selectedMode == 1) {
        int dx1 = dataVals[2] - dataVals[0]; 
        int dy1 = dataVals[3] - dataVals[1]; 
        
        if (dx1 == 0) { k1 = 999; d1 = dataVals[0]; } // Vert line
        else { k1 = dy1 / dx1; d1 = dataVals[1] - (k1 * dataVals[0]); }

        if (functionCount == 2) {
          int dx2 = dataVals[6] - dataVals[4]; 
          int dy2 = dataVals[7] - dataVals[5]; 
          if (dx2 == 0) { k2 = 999; d2 = dataVals[4]; } 
          else { k2 = dy2 / dx2; d2 = dataVals[5] - (k2 * dataVals[4]); }
        }
      }

      // --- UI RENDER ROUTINE ---
      uint8_t x = 0;
      drawString(x, 0, "f1:"); x+=18; printEqCompact(x, 0, k1, d1);
      
      if (functionCount == 2) {
        x = 0;
        drawString(x, 7, "f2:"); x+=18; printEqCompact(x, 7, k2, d2);

        if (k1 == 999 || k2 == 999) {
          drawString(x+6, 7, " (VERT)");
        } else {
          long D = (long)k1 - k2;
          if (D == 0) { drawString(x+6, 7, " (PARALLEL)"); } 
          else {
            long Dx = (long)(d2 - d1) * 100L;
            long S_x = Dx / D;
            long S_y = (k1 * Dx) / D + (d1 * 100L);
            
            drawString(x+6, 7, "S("); drawFixedPoint(x+24, 7, S_x);
            drawString(x+50, 7, "|"); drawFixedPoint(x+62, 7, S_y); drawChar(x+92, 7, ')');
          }
        }
      } else {
        drawString(0, 7, "> FIRE TO RESTART");
      }

      renderCoordinateGraph(k1, d1, k2, d2, true, (functionCount == 2));
      needsRedraw = false;
    }

    if (readFire()) {
      beep(1000, 50); appState = 2; needsRedraw = true; oled_clear();
      while(readFire()){} 
    }
  }
}