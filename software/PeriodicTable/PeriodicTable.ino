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
 * TINY ARC BUDDY - PERIODIC TABLE POCKET REFERENCE
 * ==============================================================================
 */

#include <avr/io.h>
#include <util/delay.h>

#include "font.h"     
#include "graphics.h" 
#include "elements.h"

// ==============================================================================
// ZONE 1: THE APP SETTINGS (Modify These!)
// ==============================================================================

#define OWNER_NAME "ANNA"           
#define BUILD_DATE "25.04.2026"      

#define UI_FPS 30                    
#define SCROLL_DELAY 150             

// ==============================================================================
// ZONE 2: THE ENGINE (Danger Zone!)
// ==============================================================================

#define OLED_SDA PB3  
#define OLED_SCL PB4  
#define BTN_LEFT PB0  
#define BTN_RIGHT PB2 
#define BUZZER PB1    

byte appState = 0; 
int currentElement = 0; 

unsigned long lastMoveTime = 0;
bool leftWasPressed = false;
bool rightWasPressed = false;
bool fireWasPressed = false;

// ------------------------------------------------------------------------------
// I2C BIT-BANGING ENGINE
// ------------------------------------------------------------------------------
inline void i2c_start() {
  PORTB |= (1 << OLED_SDA); PORTB |= (1 << OLED_SCL);
  PORTB &= ~(1 << OLED_SDA); PORTB &= ~(1 << OLED_SCL);
}

inline void i2c_stop() {
  PORTB &= ~(1 << OLED_SDA); PORTB |= (1 << OLED_SCL); PORTB |= (1 << OLED_SDA);
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

// ------------------------------------------------------------------------------
// UI & TEXT RENDERING
// ------------------------------------------------------------------------------
void drawChar(uint8_t x, uint8_t page, char c) {
  uint8_t idx = 0; bool isSpace = false;
  if (c >= '0' && c <= '9') idx = c - '0' + 1;
  else if (c >= 'a' && c <= 'z') idx = (c - 32) - 'A' + 11; 
  else if (c >= 'A' && c <= 'Z') idx = c - 'A' + 11;
  else if (c == '\'') idx = 37; else if (c == '@') idx = 38;
  else if (c == ':') idx = 39; else if (c == '.') idx = 40;
  else if (c == '-') idx = 41; 
  else if (c == ' ') { isSpace = true; }
  
  oled_set_cursor(x, page);
  i2c_start(); i2c_write(0x78); i2c_write(0x40);
  
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

// Reads strings directly from the Flash Memory!
void drawStringPGM(uint8_t x, uint8_t page, const char* pgmStr) {
  char c;
  while ((c = pgm_read_byte(pgmStr++)) != 0) { drawChar(x, page, c); x += 6; }
}

void drawInt(uint8_t x, uint8_t page, int val) {
  char str[6]; itoa(val, str, 10); drawString(x, page, str);
}

void drawLargeSprite(uint8_t x, uint8_t startPage, const uint8_t* sprite, uint8_t width, uint8_t pages) {
  for (uint8_t p = 0; p < pages; p++) {
    oled_set_cursor(x, startPage + p);
    i2c_start(); i2c_write(0x78); i2c_write(0x40);
    for(uint8_t i = 0; i < width; i++) i2c_write(pgm_read_byte(&sprite[(p * width) + i]));
    i2c_stop();
  }
}

void drawChar2x(uint8_t x, uint8_t startPage, char c) {
  uint8_t idx = 0;
  if (c >= 'a' && c <= 'z') idx = (c - 32) - 'A' + 11; 
  else if (c >= 'A' && c <= 'Z') idx = c - 'A' + 11;
  else return; 

  for (uint8_t p = 0; p < 2; p++) {
    oled_set_cursor(x, startPage + p);
    i2c_start(); i2c_write(0x78); i2c_write(0x40);
    
    for (uint8_t i = 0; i < 5; i++) {
      uint8_t col = pgm_read_byte(&font5x7[idx * 5 + i]);
      uint8_t doubleCol = 0;
      
      if (p == 0) {
        if (col & 0x01) doubleCol |= 0x03; if (col & 0x02) doubleCol |= 0x0C;
        if (col & 0x04) doubleCol |= 0x30; if (col & 0x08) doubleCol |= 0xC0;
      } else {
        if (col & 0x10) doubleCol |= 0x03; if (col & 0x20) doubleCol |= 0x0C;
        if (col & 0x40) doubleCol |= 0x30;
      }
      i2c_write(doubleCol); i2c_write(doubleCol);
    }
    i2c_write(0x00); i2c_write(0x00); 
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

bool readLeft() { return (PINB & (1 << BTN_LEFT)); } 
bool readRight() { return (PINB & (1 << BTN_RIGHT)); } 
bool readFire() {
  ADCSRA |= (1 << ADSC); while (ADCSRA & (1 << ADSC)); return (ADC < 950); 
}

// ------------------------------------------------------------------------------
// APPLICATION LOGIC
// ------------------------------------------------------------------------------

void renderCarousel() {
  oled_clear();
  
  // 1. Draw Nav Arrows
  drawLargeSprite(10, 3, icon_left, 8, 1);
  drawLargeSprite(110, 3, icon_right, 8, 1);

  // 2. Draw Frame Card
  drawLargeSprite(40, 1, frame_card, 48, 6);

  // 3. Draw Atomic Number
  drawInt(44, 1, currentElement + 1);

  // 4. Draw Big Symbol
  char sym1 = pgm_read_byte(&syms[currentElement][0]);
  char sym2 = pgm_read_byte(&syms[currentElement][1]);
  
  if (sym2 == '\0') drawChar2x(59, 3, sym1); // Centered
  else { drawChar2x(53, 3, sym1); drawChar2x(65, 3, sym2); }

  // 5. Draw Name
  char tempName[13];
  strcpy_P(tempName, names[currentElement]);
  int textLen = strlen(tempName) * 6;
  int startX = 64 - (textLen / 2);
  drawString(startX, 7, tempName);
}

void renderDetails() {
  oled_clear();
  
  // Line 0: Title
  drawString(0, 0, "--- DATA CARD ---");
  
  // Line 1: Name
  drawString(0, 1, "NAME: ");
  drawStringPGM(36, 1, names[currentElement]);

  // Line 2: Atomic Number
  drawString(0, 2, "NUM : ");
  drawInt(36, 2, currentElement + 1);

  // Line 3: Category (Pulled via the Dictionary Code!)
  drawString(0, 3, "TYPE: ");
  uint8_t cCode = pgm_read_byte(&catCodes[currentElement]);
  const char* catStringPtr = (const char*)pgm_read_word(&categoryDict[cCode]);
  drawStringPGM(36, 3, catStringPtr);

  // Line 4: Mass (Calculated fixed-point)
  uint32_t mass = pgm_read_dword(&masses[currentElement]);
  int wholeNum = mass / 1000;
  int fraction = mass % 1000;
  
  drawString(0, 4, "MASS: ");
  drawInt(36, 4, wholeNum);
  if (wholeNum < 10) { drawChar(42, 4, '.'); drawInt(48, 4, fraction); }
  else if (wholeNum < 100) { drawChar(48, 4, '.'); drawInt(54, 4, fraction); }
  else { drawChar(54, 4, '.'); drawInt(60, 4, fraction); }

  // Line 5: Melting Point
  drawString(0, 5, "MELT: ");
  int melt = pgm_read_word(&meltingK[currentElement]);
  drawInt(36, 5, melt);
  // Add 'K' for Kelvin after the number
  char buf[6]; itoa(melt, buf, 10);
  int len = strlen(buf);
  drawString(36 + (len*6), 5, " K");

  // Line 6: Year of Discovery
  drawString(0, 6, "YEAR: ");
  int year = pgm_read_word(&discoverYear[currentElement]);
  if (year == 0) drawString(36, 6, "ANCIENT");
  else drawInt(36, 6, year);

  // Line 7: Footer
  drawString(0, 7, "> FIRE TO EXIT <");
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

  // STATE 0: SPLASH LOGO
  if (appState == 0) {
    oled_clear();
    drawLargeSprite(48, 2, htl_logo, 32, 4); 
    drawString(25, 7, "PERIODIC TABLE");
    delay(3000); 
    appState = 1; 
  }

  // STATE 1: CREDITS
  if (appState == 1) {
    oled_clear();
    drawString(3, 0, "GIRLS' DAY"); drawString(3, 1, "@ HTL KAINDORF");
    drawString(3, 3, "BUILT BY:"); drawString(60, 3, OWNER_NAME); 
    drawString(3, 4, "ON "); drawString(25, 4, BUILD_DATE);  
    
    drawString(2, 6, "> PRESS FIRE TO OPEN");
    
    while(!readFire()) { /* Wait */ }
    beep(1000, 100);
    
    appState = 2; 
    renderCarousel();
    lastMoveTime = millis();
  }

  // STATE 2: CAROUSEL VIEW
  if (appState == 2) {
    bool leftNow = readLeft();
    if (leftNow && (!leftWasPressed || currentMillis - lastMoveTime > SCROLL_DELAY)) {
      if (currentElement > 0) {
        currentElement--;
        beep(2000, 10);
        renderCarousel();
      } else {
        beep(100, 30); 
      }
      lastMoveTime = currentMillis;
    }
    leftWasPressed = leftNow;

    bool rightNow = readRight();
    if (rightNow && (!rightWasPressed || currentMillis - lastMoveTime > SCROLL_DELAY)) {
      if (currentElement < ELEMENT_COUNT - 1) {
        currentElement++;
        beep(2000, 10);
        renderCarousel();
      } else {
        beep(100, 30); 
      }
      lastMoveTime = currentMillis;
    }
    rightWasPressed = rightNow;

    bool fireNow = readFire();
    if (fireNow && !fireWasPressed) {
      beep(1500, 50);
      appState = 3;
      renderDetails();
    }
    fireWasPressed = fireNow;
  }

  // STATE 3: DETAILS VIEW
  if (appState == 3) {
    bool fireNow = readFire();
    if (fireNow && !fireWasPressed) {
      beep(1000, 50);
      appState = 2;
      renderCarousel(); 
    }
    fireWasPressed = fireNow;
    delay(10); 
  }
}