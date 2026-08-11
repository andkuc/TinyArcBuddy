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
 * TINY ARC BUDDY - TETRIS EDITION (ARTIFACT-FREE UPDATE)
 * ==============================================================================
 */

#include <avr/io.h>
#include <util/delay.h>
#include <EEPROM.h>

#include "graphics.h"
#include "font.h"     
#include "shapes.h" 

// ==============================================================================
// ZONE 1: THE GAME RULES (Modify These!)
// ==============================================================================

#define PLAYER_NAME "ANNA"           
#define BUILD_DATE "25.04.2026"      

// --- Game Pacing & Rules ---
#define STARTING_SPEED 800           // Milliseconds between block drops
#define SPEED_UP_AMOUNT 50           // Speed up by this much every level
#define POINTS_PER_LINE 100          // Base points for clearing 1 line
#define POINTS_MULTIPLIER 50         // Extra bonus for doing 2, 3, or 4 lines at once!

// --- Music Studio ---
const int levelUpMelody[] PROGMEM = { 440, 554, 659, 880 }; 
const int levelUpTempo[]  PROGMEM = { 100, 100, 100, 300 };
const int gameOverMelody[] PROGMEM = { 300, 250, 200, 150 };
const int gameOverTempo[]  PROGMEM = { 300, 300, 300, 600 };

// ==============================================================================
// ZONE 2: THE ENGINE (Danger Zone!)
// ==============================================================================

// --- Hardware Pins ---
#define OLED_SDA PB3  
#define OLED_SCL PB4  
#define BTN_LEFT PB0  
#define BTN_RIGHT PB2 
#define BUZZER PB1    
// PB5 (Reset) is Analog FIRE button


// --- Game State ---
byte gameState = 0; 
int highScore = 0;
int score = 0;
int linesCleared = 0;
int level = 1;

uint16_t board[16]; 
int pieceX = 3, pieceY = 0;
int pieceType = 0, pieceRot = 0;
int nextPiece = 0;

unsigned long lastDropTime = 0;
unsigned long lastMoveTime = 0;
bool leftWasPressed = false;
bool rightWasPressed = false;
bool fireWasPressed = false;

// ------------------------------------------------------------------------------
// I2C BIT-BANGING ENGINE (ARTIFACT-FREE UPDATE)
// ------------------------------------------------------------------------------
// These explicit functions fix the hardware race conditions that caused stray lines!
inline void i2c_start() {
  PORTB |= (1 << OLED_SDA);
  PORTB |= (1 << OLED_SCL);
  PORTB &= ~(1 << OLED_SDA);
  PORTB &= ~(1 << OLED_SCL);
}

inline void i2c_stop() {
  PORTB &= ~(1 << OLED_SDA);
  PORTB |= (1 << OLED_SCL);
  PORTB |= (1 << OLED_SDA);
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
  i2c_start();
  i2c_write(0x78); i2c_write(0x00); i2c_write(cmd);
  i2c_stop();
}

void oled_set_cursor(uint8_t x, uint8_t y) {
  oled_command(0xB0 + y);                 
  oled_command(0x00 + (x & 0x0F));        
  oled_command(0x10 + ((x >> 4) & 0x0F)); 
}

void oled_clear() {
  for (uint8_t y = 0; y < 8; y++) {
    oled_set_cursor(0, y);
    i2c_start();
    i2c_write(0x78); i2c_write(0x40);
    for (uint8_t x = 0; x < 128; x++) i2c_write(0x00);
    i2c_stop();
  }
}

// ------------------------------------------------------------------------------
// UI & SOUND FUNCTIONS
// ------------------------------------------------------------------------------
void drawChar(uint8_t x, uint8_t page, char c) {
  uint8_t idx = 0; bool isSpace = false;
  if (c >= '0' && c <= '9') idx = c - '0' + 1;
  else if (c >= 'A' && c <= 'Z') idx = c - 'A' + 11;
  else if (c == '\'') idx = 37; else if (c == '@') idx = 38;
  else if (c == ':') idx = 39; else if (c == '.') idx = 40;
  else if (c == ' ') { isSpace = true; }
  
  oled_set_cursor(x, page);
  i2c_start();
  i2c_write(0x78); i2c_write(0x40);
  
  if (isSpace) for(uint8_t i=0; i<6; i++) i2c_write(0x00);
  else {
    for(uint8_t i=0; i<5; i++) i2c_write(pgm_read_byte(&font5x7[idx * 5 + i]));
    i2c_write(0x00); 
  }
  i2c_stop();
}

void drawString(uint8_t x, uint8_t page, const char* str) {
  while (*str) { drawChar(x, page, *str++); x += 6; }
}

void drawInt(uint8_t x, uint8_t page, int val) {
  char str[6]; itoa(val, str, 10); drawString(x, page, str);
}

void drawLargeSprite(uint8_t x, uint8_t startPage, const uint8_t* sprite, uint8_t width, uint8_t pages) {
  for (uint8_t p = 0; p < pages; p++) {
    oled_set_cursor(x, startPage + p);
    i2c_start();
    i2c_write(0x78); i2c_write(0x40);
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

void playSong(const int* notes, const int* durations, int length) {
  for(int i=0; i<length; i++) {
    int n = pgm_read_word(&notes[i]); int d = pgm_read_word(&durations[i]);
    if (n > 0) beep(n, d); else delay(d);
    delay(20); 
  }
}

bool readLeft() { return (PINB & (1 << BTN_LEFT)); } 
bool readRight() { return (PINB & (1 << BTN_RIGHT)); } 
bool readFire() {
  ADCSRA |= (1 << ADSC); while (ADCSRA & (1 << ADSC)); return (ADC < 950); 
}

// ------------------------------------------------------------------------------
// TETRIS ENGINE
// ------------------------------------------------------------------------------
bool checkCollision(int tryX, int tryY, int tryRot, int type) {
  uint16_t shape = pgm_read_word(&shapes[type][tryRot]);
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if ((shape >> ((3-r)*4 + (3-c))) & 1) { 
        int bx = tryX + c; int by = tryY + r;
        if (bx < 0 || bx >= 10 || by >= 16) return true; 
        if (by >= 0 && bitRead(board[by], bx)) return true; 
      }
    }
  }
  return false;
}

void renderUI() {
  drawString(2, 1, "SCORE"); drawInt(2, 2, score);
  drawString(2, 5, "LEVEL"); drawInt(2, 6, level);
  drawString(90, 1, "NEXT");
}

void drawNextPiece() {
  uint16_t shape = pgm_read_word(&shapes[nextPiece][0]);
  for (int p = 1; p < 3; p++) {
    oled_set_cursor(90, p + 1); 
    i2c_start();
    i2c_write(0x78); i2c_write(0x40);

    for (int c = 0; c < 4; c++) {
      uint8_t data = 0x00;
      if ((shape >> ((3-(p*2-2))*4 + (3-c))) & 1) data |= 0x07;
      if ((shape >> ((3-(p*2-1))*4 + (3-c))) & 1) data |= 0x70;
      i2c_write(data); i2c_write(data); i2c_write(data); i2c_write(0x00);
    }
    i2c_stop();
  }
}

void renderPlayfield() {
  int ghostY = pieceY;
  while (!checkCollision(pieceX, ghostY + 1, pieceRot, pieceType)) ghostY++;
  uint16_t shape = pgm_read_word(&shapes[pieceType][pieceRot]);

  for (uint8_t p = 0; p < 8; p++) {
    // 42 centers perfectly! 2px wall + 40px board + 2px wall = 44px total width.
    oled_set_cursor(42, p); 
    i2c_start();
    i2c_write(0x78); i2c_write(0x40);

    i2c_write(0xFF); i2c_write(0xFF); // Thick Left Wall

    for (uint8_t c = 0; c < 10; c++) {
      bool topOcc = bitRead(board[p*2], c);
      bool botOcc = bitRead(board[p*2 + 1], c);
      bool topGho = false, botGho = false;

      int prTop = p*2 - pieceY;     int prBot = (p*2 + 1) - pieceY;
      int grTop = p*2 - ghostY;     int grBot = (p*2 + 1) - ghostY;

      if (prTop >= 0 && prTop < 4 && c >= pieceX && c < pieceX + 4 && ((shape >> ((3-prTop)*4 + (3-(c-pieceX)))) & 1)) topOcc = true;
      if (prBot >= 0 && prBot < 4 && c >= pieceX && c < pieceX + 4 && ((shape >> ((3-prBot)*4 + (3-(c-pieceX)))) & 1)) botOcc = true;

      if (grTop >= 0 && grTop < 4 && c >= pieceX && c < pieceX + 4 && ((shape >> ((3-grTop)*4 + (3-(c-pieceX)))) & 1)) topGho = true;
      if (grBot >= 0 && grBot < 4 && c >= pieceX && c < pieceX + 4 && ((shape >> ((3-grBot)*4 + (3-(c-pieceX)))) & 1)) botGho = true;

      uint8_t data = 0x00;
      if (topOcc) data |= 0x07; else if (topGho) data |= 0x05; 
      if (botOcc) data |= 0x70; else if (botGho) data |= 0x50; 

      i2c_write(data); i2c_write(data); i2c_write(data); i2c_write(0x00); 
    }
    i2c_write(0xFF); i2c_write(0xFF); // Thick Right Wall
    i2c_stop();
  }
}

void lockPiece() {
  uint16_t shape = pgm_read_word(&shapes[pieceType][pieceRot]);
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if ((shape >> ((3-r)*4 + (3-c))) & 1) {
        if (pieceY + r >= 0) bitSet(board[pieceY + r], pieceX + c);
      }
    }
  }

  int linesNow = 0;
  for (int r = 15; r >= 0; r--) {
    bool full = true;
    for (int c = 0; c < 10; c++) if (!bitRead(board[r], c)) { full = false; break; }
    
    if (full) {
      linesNow++;
      for (int i = r; i > 0; i--) board[i] = board[i-1];
      board[0] = 0;
      r++; 
    }
  }

  if (linesNow > 0) {
    beep(1500, 100);
    linesCleared += linesNow;
    score += (POINTS_PER_LINE * linesNow) + (POINTS_MULTIPLIER * linesNow * linesNow);
    if (linesCleared >= level * 10) {
      level++;
      playSong(levelUpMelody, levelUpTempo, 4);
    }
    renderUI();
  } else {
    beep(400, 20); 
  }

  pieceX = 3; pieceY = 0; pieceRot = 0;
  pieceType = nextPiece;
  nextPiece = random(7);
  drawNextPiece();

  if (checkCollision(pieceX, pieceY, pieceRot, pieceType)) gameState = 3; 
}

void setup() {
  DDRB |= (1 << OLED_SDA) | (1 << OLED_SCL) | (1 << BUZZER);
  DDRB &= ~((1 << BTN_LEFT) | (1 << BTN_RIGHT));
  PORTB &= ~((1 << BTN_LEFT) | (1 << BTN_RIGHT)); 
  ADMUX = 0b00000000; ADCSRA = 0b10000011; 

  oled_command(0xAE); oled_command(0x20); oled_command(0x02); 
  oled_command(0xA1); oled_command(0xC8); oled_command(0x8D); 
  oled_command(0x14); oled_command(0xAF); 
  
  EEPROM.get(0, highScore);
  if (highScore == -1 || highScore > 9999) highScore = 0; 
}

void loop() {
  unsigned long currentMillis = millis();

  // ----------------------------------------------------------------
  // STATE 0: HTL LOGO SPLASH
  // ----------------------------------------------------------------
  if (gameState == 0) {
    oled_clear();
    drawLargeSprite(48, 2, htl_logo, 32, 4); 
    drawString(25, 7, "TETRIS CLONE");
    delay(3000); 
    gameState = 1; 
  }

  // ----------------------------------------------------------------
  // STATE 1: CREDITS & HIGH SCORE
  // ----------------------------------------------------------------
  if (gameState == 1) {
    oled_clear();
    drawString(3, 0, "GIRLS' DAY"); drawString(3, 1, "@ HTL KAINDORF");
    drawString(3, 3, "BUILT BY:"); drawString(60, 3, PLAYER_NAME); 
    drawString(3, 4, "ON "); drawString(25, 4, BUILD_DATE);  
    drawString(3, 6, "HIGH SCORE:"); drawInt(75, 6, highScore);
    
    while(!readFire()) { /* Wait */ }
    
    // Human Entropy for perfectly random block drops!
    randomSeed(micros()); 
    beep(1000, 100);
    
    score = 0; linesCleared = 0; level = 1;
    for(int i=0; i<16; i++) board[i] = 0;
    
    pieceType = random(7); nextPiece = random(7);
    pieceX = 3; pieceY = 0; pieceRot = 0;
    
    oled_clear();
    renderUI(); drawNextPiece();
    lastDropTime = millis(); lastMoveTime = millis();
    gameState = 2; 
  }

  // ----------------------------------------------------------------
  // STATE 2: PLAYING
  // ----------------------------------------------------------------
  if (gameState == 2) {
    
    bool leftNow = readLeft();
    if (leftNow && (!leftWasPressed || currentMillis - lastMoveTime > 150)) {
      if (!checkCollision(pieceX - 1, pieceY, pieceRot, pieceType)) { pieceX--; beep(800, 5); }
      lastMoveTime = currentMillis;
    }
    leftWasPressed = leftNow;

    bool rightNow = readRight();
    if (rightNow && (!rightWasPressed || currentMillis - lastMoveTime > 150)) {
      if (!checkCollision(pieceX + 1, pieceY, pieceRot, pieceType)) { pieceX++; beep(800, 5); }
      lastMoveTime = currentMillis;
    }
    rightWasPressed = rightNow;

    bool fireNow = readFire();
    if (fireNow && !fireWasPressed) {
      int newRot = (pieceRot + 1) % 4;
      if (!checkCollision(pieceX, pieceY, newRot, pieceType)) { pieceRot = newRot; beep(1200, 5); }
    }
    fireWasPressed = fireNow;

    int currentDropSpeed = STARTING_SPEED - (level * SPEED_UP_AMOUNT);
    if (currentDropSpeed < 100) currentDropSpeed = 100; 

    if (currentMillis - lastDropTime > currentDropSpeed) {
      if (!checkCollision(pieceX, pieceY + 1, pieceRot, pieceType)) {
        pieceY++;
      } else {
        lockPiece();
      }
      lastDropTime = currentMillis;
    }

    renderPlayfield();
  }

  // ----------------------------------------------------------------
  // STATE 3: GAME OVER
  // ----------------------------------------------------------------
  if (gameState == 3) {
    if (score > highScore) { highScore = score; EEPROM.put(0, highScore); }

    oled_clear();
    drawString(35, 2, "GAME OVER");
    drawString(30, 4, "SCORE: "); drawInt(70, 4, score);
    
    playSong(gameOverMelody, gameOverTempo, 4);
    delay(1000);
    drawString(20, 6, "PRESS FIRE");
    
    while(!readFire()) { /* Wait */ }
    delay(300); gameState = 0; 
  }
}