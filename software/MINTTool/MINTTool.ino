/*
 * =========================================================================================
 * TINY ARC BUDDY - EDUCATIONAL EMBEDDED PLATFORM
 * =========================================================================================
 * * Copyright (c) 2026 Andreas Kucher
 * * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * * -----------------------------------------------------------------------------------------
 * ACKNOWLEDGMENTS & DISCLAIMER
 * -----------------------------------------------------------------------------------------
 * This project was designed for STEM education workshops.
 * * Core hardware architecture derived from:
 * "Tiny Arcade Game (ATtiny85)" - Instructables
 * * Hardware support via:
 * ATTinyCore by Spence Konde (https://github.com/SpenceKonde/ATTinyCore)
 * * Disclaimer: The software payloads in this repository were heavily optimized 
 * for ATtiny85 SRAM/Flash constraints with the assistance of AI (Google Gemini). 
 * They have not undergone exhaustive edge-case testing. Use for educational purposes.
 * =========================================================================================
 */

/*
 * ==============================================================================
 * TINY ARC BUDDY - GENIUS EDITION (Flicker-Free, Live Equations)
 * ==============================================================================
 */

#include <avr/io.h>
#include <util/delay.h>

#include "font.h"     
#include "graphics.h" 
#include "formulas.h"

// ==============================================================================
// ZONE 1: APP SETTINGS
// ==============================================================================
#define OWNER_NAME "ANNA"           
#define BUILD_DATE "25.04.2026"      

#define OLED_SDA PB3  
#define OLED_SCL PB4  
#define BTN_LEFT PB0  
#define BTN_RIGHT PB2 
#define BUZZER PB1    

byte appState = 0; 
bool needsRedraw = true; // Our flicker-prevention flag!

int activeCat = 0;
int activeTool = 0;
int activeUnitGroup = 0;
int selectedUnitIdx = 0;

int inputVals[6];     
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
  oled_command(0xB0 + y);                 
  oled_command(0x00 + (x & 0x0F));        
  oled_command(0x10 + ((x >> 4) & 0x0F)); 
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
  else if (c == '(' || c == '[') idx = 43; 
  else if (c == ')' || c == ']') idx = 44;
  else if (c == '^') idx = 45; else if (c == '%') idx = 46;
  else if (c == '+') idx = 47; else if (c == '*') idx = 48;
  else if (c == '/') idx = 49; else if (c == ',') idx = 50;
  else if (c == '_') idx = 51; else if (c == ' ') { isSpace = true; }
  
  oled_set_cursor(x, page);
  i2c_start(); i2c_write(0x78); i2c_write(0x40);
  
  if (isSpace || idx == 0) for(uint8_t i=0; i<6; i++) i2c_write(0x00);
  else {
    for(uint8_t i=0; i<5; i++) i2c_write(pgm_read_byte(&font5x7[idx * 5 + i]));
    i2c_write(0x00); 
  }
  i2c_stop();
}

void drawString(uint8_t x, uint8_t page, const char* str) {
  while (*str) { drawChar(x, page, *str++); x += 6; }
}

void drawStringPGM(uint8_t x, uint8_t page, const char* pgmStr) {
  char c;
  while ((c = pgm_read_byte(pgmStr++)) != 0) { drawChar(x, page, c); x += 6; }
}

// THE FLICKER KILLER: Erases leftover pixels on a line instantly!
void clearRestOfLine(uint8_t x, uint8_t page) {
  while (x < 124) { drawChar(x, page, ' '); x += 6; }
}

// Live-updating Equation Element (e.g., "[ 12]x" or "  5 y")
void drawEqElem(uint8_t &x, uint8_t page, int val, bool isActive, char varName) {
  if (isActive) { drawChar(x, page, '['); x+=6; }
  else { drawChar(x, page, ' '); x+=6; } // Spaces align perfectly!

  char buf[8]; itoa(val, buf, 10);
  drawString(x, page, buf); x+= strlen(buf)*6;

  if (isActive) { drawChar(x, page, ']'); x+=6; }
  else { drawChar(x, page, ' '); x+=6; }

  if (varName != ' ') { drawChar(x, page, varName); x+=6; }
}

void drawFixedPoint(uint8_t x, uint8_t page, long val) {
  if (val < 0) { drawChar(x, page, '-'); x += 6; val = -val; }
  long whole = val / 100;
  int frac = val % 100;
  
  char buf[12]; itoa(whole, buf, 10);
  int len = strlen(buf);
  buf[len] = '.'; buf[len+1] = (frac / 10) + '0';
  buf[len+2] = (frac % 10) + '0'; buf[len+3] = '\0';
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

// Memory-saving integer square root
long isqrt(long n) {
  if (n <= 0) return 0;
  long root = 0, bit = 1L << 30;
  while (bit > n) bit >>= 2;
  while (bit != 0) {
    if (n >= root + bit) { n -= root + bit; root = (root >> 1) + bit; } 
    else root >>= 1;
    bit >>= 2;
  }
  return root;
}

void beep(int freq, int duration) {
  int halfPeriod = 1000000L / freq / 2;
  long loops = (long)duration * 1000L / (halfPeriod * 2);
  for (long i=0; i<loops; i++) {
    PORTB |= (1 << BUZZER); delayMicroseconds(halfPeriod);
    PORTB &= ~(1 << BUZZER); delayMicroseconds(halfPeriod);
  }
}

bool readLeft() { return (PINB & (1 << BTN_LEFT)); } 
bool readRight() { return (PINB & (1 << BTN_RIGHT)); } 
bool readFire() {
  ADCSRA |= (1 << ADSC); while (ADCSRA & (1 << ADSC)); return (ADC < 950); 
}

// ------------------------------------------------------------------------------
// APPLICATION LOGIC
// ------------------------------------------------------------------------------

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

  // ----------------------------------------------------------------
  // STATE 0: HTL LOGO SPLASH
  // ----------------------------------------------------------------
  if (appState == 0) {
    oled_clear();
    drawLargeSprite(48, 2, htl_logo, 32, 4); 
    drawString(37, 7, "MINT Tool");
    delay(3000); 
    appState = 1; needsRedraw = true; oled_clear();
  }

  // ----------------------------------------------------------------
  // STATE 1: CREDITS
  // ----------------------------------------------------------------
  if (appState == 1) {
    if (needsRedraw) {
      drawString(3, 0, "GIRLS' DAY"); drawString(3, 1, "@ HTL KAINDORF");
      drawString(3, 3, "BUILT BY:"); drawString(60, 3, OWNER_NAME); 
      drawString(3, 4, "ON "); drawString(25, 4, BUILD_DATE);  
      drawString(2, 7, "> PRESS FIRE TO OPEN");
      needsRedraw = false;
    }
    if (readFire()) {
      beep(1000, 100); appState = 2; needsRedraw = true; oled_clear();
      while(readFire()){ delay(10); }
    }
  }

  // ----------------------------------------------------------------
  // STATE 2: CATEGORY MENU
  // ----------------------------------------------------------------
  if (appState == 2) {
    if (needsRedraw) {
      drawString(10, 1, "SELECT CATEGORY:");
      uint8_t x = 15;
      const char* catStr = (const char*)pgm_read_word(&catStrings[activeCat]);
      drawStringPGM(x, 4, catStr);
      x += strlen_P(catStr) * 6;
      clearRestOfLine(x, 4); // Clear any leftover text instantly!
      needsRedraw = false;
    }
    
    if (readLeft() && !leftWasPressed) {
      if (activeCat > 0) activeCat--; else activeCat = 2;
      beep(1000, 20); needsRedraw = true;
    } leftWasPressed = readLeft();

    if (readRight() && !rightWasPressed) {
      if (activeCat < 2) activeCat++; else activeCat = 0;
      beep(1000, 20); needsRedraw = true;
    } rightWasPressed = readRight();

    if (readFire() && !fireWasPressed) {
      beep(1500, 50);
      for (int i=0; i<14; i++) {
        if (pgm_read_byte(&toolCats[i]) == activeCat) { activeTool = i; break; }
      }
      appState = 3; needsRedraw = true; oled_clear(); delay(200);
    } fireWasPressed = readFire();
  }

  // ----------------------------------------------------------------
  // STATE 3: TOOL SELECT
  // ----------------------------------------------------------------
  if (appState == 3) {
    if (needsRedraw) {
      drawString(10, 1, "SELECT TOOL:");
      uint8_t x = 15;
      const char* toolStr = (const char*)pgm_read_word(&toolStrings[activeTool]);
      drawStringPGM(x, 4, toolStr);
      x += strlen_P(toolStr) * 6;
      clearRestOfLine(x, 4);
      needsRedraw = false;
    }
    
    if (readLeft() && !leftWasPressed) {
      do { if (activeTool > 0) activeTool--; else activeTool = 13; } 
      while (pgm_read_byte(&toolCats[activeTool]) != activeCat);
      beep(1000, 20); needsRedraw = true;
    } leftWasPressed = readLeft();

    if (readRight() && !rightWasPressed) {
      do { if (activeTool < 13) activeTool++; else activeTool = 0; } 
      while (pgm_read_byte(&toolCats[activeTool]) != activeCat);
      beep(1000, 20); needsRedraw = true;
    } rightWasPressed = readRight();

    if (readFire() && !fireWasPressed) {
      beep(2000, 50);
      activeUnitGroup = pgm_read_byte(&toolUnitGroup[activeTool]);
      
      // Zero out equation memory to prevent "weird placeholders"
      for(int i=0; i<6; i++) inputVals[i] = 0;

      if (activeUnitGroup == 3) {
        inputStep = 0; currentInput = 0; appState = 5; 
      } else {
        selectedUnitIdx = 0; appState = 4;
      }
      needsRedraw = true; oled_clear(); delay(200);
    } fireWasPressed = readFire();
  }

  // ----------------------------------------------------------------
  // STATE 4: UNIT SELECTION
  // ----------------------------------------------------------------
  if (appState == 4) {
    if (needsRedraw) {
      drawString(10, 1, "SELECT UNIT:");
      uint8_t gSize = pgm_read_byte(&groupSizes[activeUnitGroup]);
      const char* const* gArray = (const char* const*)pgm_read_word(&unitGroups[activeUnitGroup]);
      const char* uStr = (const char*)pgm_read_word(&gArray[selectedUnitIdx]);
      
      drawChar(40, 4, '[');
      uint8_t uX = 46; 
      drawStringPGM(uX, 4, uStr);
      uX += strlen_P(uStr) * 6;
      drawChar(uX, 4, ']'); uX += 6;
      clearRestOfLine(uX, 4);
      needsRedraw = false;
    }
    
    if (readLeft() && !leftWasPressed) {
      if (selectedUnitIdx > 0) selectedUnitIdx--; else selectedUnitIdx = pgm_read_byte(&groupSizes[activeUnitGroup]) - 1;
      beep(1000, 20); needsRedraw = true;
    } leftWasPressed = readLeft();

    if (readRight() && !rightWasPressed) {
      if (selectedUnitIdx < pgm_read_byte(&groupSizes[activeUnitGroup]) - 1) selectedUnitIdx++; else selectedUnitIdx = 0;
      beep(1000, 20); needsRedraw = true;
    } rightWasPressed = readRight();

    if (readFire() && !fireWasPressed) {
      beep(2000, 50);
      inputStep = 0; currentInput = 0;
      appState = 5; needsRedraw = true; oled_clear(); delay(200);
    } fireWasPressed = readFire();
  }

  // ----------------------------------------------------------------
  // STATE 5: NUMBER INPUT ENGINE (Live Rendering)
  // ----------------------------------------------------------------
  if (appState == 5) {
    if (needsRedraw) {
      if (activeTool == 10) {
        // --- 2x2 EQUATION UI ---
        drawString(10, 0, "2x2 LINEAR SYS:");
        
        uint8_t cx = 5;
        drawEqElem(cx, 3, (inputStep==0?currentInput:inputVals[0]), inputStep==0, 'x');
        drawChar(cx, 3, '+'); cx+=6;
        drawEqElem(cx, 3, (inputStep==1?currentInput:inputVals[1]), inputStep==1, 'y');
        drawChar(cx, 3, '='); cx+=6;
        drawEqElem(cx, 3, (inputStep==2?currentInput:inputVals[2]), inputStep==2, ' ');
        clearRestOfLine(cx, 3);

        cx = 5;
        drawEqElem(cx, 5, (inputStep==3?currentInput:inputVals[3]), inputStep==3, 'x');
        drawChar(cx, 5, '+'); cx+=6;
        drawEqElem(cx, 5, (inputStep==4?currentInput:inputVals[4]), inputStep==4, 'y');
        drawChar(cx, 5, '='); cx+=6;
        drawEqElem(cx, 5, (inputStep==5?currentInput:inputVals[5]), inputStep==5, ' ');
        clearRestOfLine(cx, 5);

      } else {
        // --- STANDARD PROMPT UI ---
        const char* const* promptList = (const char* const*)pgm_read_word(&toolPrompts[activeTool]);
        const char* activePrompt = (const char*)pgm_read_word(&promptList[inputStep]);
        drawString(10, 1, "ENTER VALUE:");
        
        uint8_t px = 10; 
        drawStringPGM(px, 3, activePrompt);
        px += strlen_P(activePrompt) * 6;
        
        if (activeUnitGroup != 3) {
          const char* const* gArray = (const char* const*)pgm_read_word(&unitGroups[activeUnitGroup]);
          const char* uStr = (const char*)pgm_read_word(&gArray[selectedUnitIdx]);
          drawStringPGM(px, 3, uStr);
          px += strlen_P(uStr) * 6;
        }
        clearRestOfLine(px, 3);
        
        uint8_t bx = 25; drawChar(bx, 5, '['); bx += 6;
        char buf[8]; itoa(currentInput, buf, 10);
        drawString(bx, 5, buf); bx += strlen(buf) * 6;
        drawChar(bx, 5, ']'); bx += 6;
        clearRestOfLine(bx, 5); 
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
      inputVals[inputStep] = currentInput;
      inputStep++; currentInput = 0; needsRedraw = true; delay(200);

      if (inputStep >= pgm_read_byte(&toolInputCounts[activeTool])) {
        appState = 6; oled_clear();
      }
    } fireWasPressed = readFire();
  }

 // ----------------------------------------------------------------
  // STATE 6: THE MATH ENGINE (Output)
  // ----------------------------------------------------------------
  if (appState == 6) {
    if (needsRedraw) {
      drawString(0, 0, "--- RESULT ---");
      long res1 = 0, res2 = 0;
      bool hasRes2 = false, error = false;

      // --- 1. GEOMETRY ---
      if (activeTool == 0) { 
        // Cylinder Vol: V = pi * r^2 * h (Scaled 3.14 -> 314L)
        res1 = 314L * inputVals[0] * inputVals[0] * inputVals[1];
        drawString(5, 3, "Vol V ="); 
      } 
      else if (activeTool == 1) { 
        // Hypotenuse: c = sqrt(a^2 + b^2). 
        // Scale by 10000 BEFORE the root to preserve 2 decimal places!
        res1 = isqrt(((long)inputVals[0]*inputVals[0] + (long)inputVals[1]*inputVals[1]) * 10000L);
        drawString(5, 3, "Side c ="); 
      } 
      else if (activeTool == 2) { 
        // Circle Area: A = pi * r^2
        res1 = 314L * inputVals[0] * inputVals[0];
        drawString(5, 3, "Area A ="); 
      } 
      else if (activeTool == 3) { 
        // Sphere Vol: V = 4/3 * pi * r^3 (4/3 * 3.14 = 4.1887 -> 419L)
        res1 = 419L * inputVals[0] * inputVals[0] * inputVals[0];
        drawString(5, 3, "Vol V ="); 
      } 
      else if (activeTool == 4) { 
        // Rect Area: A = a * b (Scale output by 100L)
        res1 = (long)inputVals[0] * inputVals[1] * 100L;
        drawString(5, 3, "Area A ="); 
      } 
      
      // --- 2. PHYSICS ---
      else if (activeTool == 5) { 
        if (inputVals[1] == 0) error = true;
        else res1 = ((long)inputVals[0] * 100L) / inputVals[1];
        drawString(5, 3, "Speed v =");
      } 
      else if (activeTool == 6) { 
        res1 = (long)inputVals[0] * inputVals[1] * 100L;
        drawString(5, 3, "Volt U ="); 
      } 
      else if (activeTool == 7) { 
        if (inputVals[1] == 0) error = true;
        else res1 = ((long)inputVals[0] * 100L) / inputVals[1];
        drawString(5, 3, "Dens. p =");
      } 
      else if (activeTool == 8) { 
        res1 = (long)inputVals[0] * inputVals[1] * 100L;
        drawString(5, 3, "Work W ="); 
      } 
      else if (activeTool == 9) { 
        // Kinetic Energy: E = 0.5 * m * v^2 (0.5 scaled is 50L)
        res1 = 50L * inputVals[0] * inputVals[1] * inputVals[1];
        drawString(5, 3, "Energy E="); 
      } 
      
      // --- 3. ALGEBRA ---
      else if (activeTool == 10) { 
        long D = (long)inputVals[0]*inputVals[4] - (long)inputVals[3]*inputVals[1];
        if (D == 0) error = true;
        else {
          long Dx = (long)inputVals[2]*inputVals[4] - (long)inputVals[5]*inputVals[1];
          long Dy = (long)inputVals[0]*inputVals[5] - (long)inputVals[3]*inputVals[2];
          res1 = (Dx * 100L) / D; res2 = (Dy * 100L) / D;
          hasRes2 = true;
          drawString(10, 2, "X = "); drawString(10, 4, "Y = ");
        }
      }
      else if (activeTool == 11) { 
        // X% of Y = (X * Y) / 100. Because we want it scaled by 100, we simply multiply.
        res1 = (long)inputVals[0] * inputVals[1];
        drawString(5, 3, "Result ="); 
      }
      else if (activeTool == 12) { 
        if (inputVals[1] == 0) error = true;
        else res1 = ((long)inputVals[0] * 100L) / inputVals[1];
        drawString(5, 3, "Decimal =");
      }
      else if (activeTool == 13) {
        long disc = (long)inputVals[1]*inputVals[1] - (4L*inputVals[0]*inputVals[2]);
        if (disc < 0 || inputVals[0] == 0) error = true;
        else {
          // Scale discriminant by 10,000 BEFORE the root to preserve 2 decimal places
          long root = isqrt(disc * 10000L); 
          res1 = ((-inputVals[1]*100L) + root) / (2L * inputVals[0]);
          res2 = ((-inputVals[1]*100L) - root) / (2L * inputVals[0]);
          hasRes2 = true;
          drawString(10, 2, "X1 = "); drawString(10, 4, "X2 = ");
        }
      }

      // --- OUTPUT PRINTING ---
      if (error) {
        drawString(10, 3, "MATH ERROR!");
        drawString(10, 4, "(DIV BY ZERO/NEG)");
      } else {
        if (hasRes2) {
          drawFixedPoint(40, 2, res1);
          drawFixedPoint(40, 4, res2);
        } else {
          drawFixedPoint(65, 3, res1);
          if (activeUnitGroup != 3) {
            const char* const* gArray = (const char* const*)pgm_read_word(&unitGroups[activeUnitGroup]);
            const char* uStr = (const char*)pgm_read_word(&gArray[selectedUnitIdx]);
            
            drawString(10, 5, "Unit: ");
            uint8_t ux = 46; drawStringPGM(ux, 5, uStr);
            ux += strlen_P(uStr) * 6;
            if (activeTool == 0 || activeTool == 3) drawString(ux, 5, "^3");
            if (activeTool == 2 || activeTool == 4) drawString(ux, 5, "^2");
          } else {
            if (activeTool == 6) drawString(10, 5, "(Volts)");
            if (activeTool == 8 || activeTool == 9) drawString(10, 5, "(Joules)");
          }
        }
      }
      drawString(0, 7, "> FIRE TO MENU");
      needsRedraw = false;
    }

    if (readFire()) {
      beep(1000, 50); appState = 2;
      needsRedraw = true; oled_clear();
      while(readFire()){ delay(10); }
    }
  }
}