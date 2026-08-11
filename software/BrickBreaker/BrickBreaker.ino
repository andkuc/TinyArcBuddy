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
 * TINY ARC BUDDY - BRICK BREAKER EDITION (PERFORMANCE UPDATE)
 * ==============================================================================
 */

#include <avr/io.h>
#include <util/delay.h>
#include <EEPROM.h>

#include "font.h"     
#include "graphics.h" 
#include "levels.h"

// ==============================================================================
// ZONE 1: THE GAME RULES (Modify These!)
// ==============================================================================

#define PLAYER_NAME "ANNA"           
#define BUILD_DATE "25.04.2026"      

#define STARTING_LIVES 3             
#define BRICK_POINTS 10              
#define PADDLE_SPEED 4               // Increased speed for snappy controls!

// --- Global Game Pacing ---
#define GAME_FPS 30                  
#define BASE_BALL_SPEED 8            

// --- Music Studio ---
const int levelUpMelody[] PROGMEM = { 440, 554, 659, 880 }; 
const int levelUpTempo[]  PROGMEM = { 100, 100, 100, 300 };

const int gameOverMelody[] PROGMEM = { 300, 250, 200, 150 };
const int gameOverTempo[]  PROGMEM = { 300, 300, 300, 600 };

// ==============================================================================
// ZONE 2: THE ENGINE (Danger Zone!)
// ==============================================================================

// RESTORED PROVEN HARDWARE PINS!
#define OLED_SDA PB3  
#define OLED_SCL PB4  
#define BTN_LEFT PB0  
#define BTN_RIGHT PB2 
#define BUZZER PB1    

// --- Game State ---
byte gameState = 0; 
int highScore = 0;
int score = 0;
int lives = STARTING_LIVES;
int level = 1;

byte currentLevel[BRICK_ROWS * BRICK_COLS]; 
int bricksRemaining = 0;

int paddleX = 56; 
int paddleMode = 0;   
int paddleTimer = 0;  

bool modActive = false;
int modX = -1;
int modPage = -1;
int modTick = 0;

// Fixed-Point Physics Engine
int ballX = 640; 
int ballY = 500;
int ballDx = 8;  
int ballDy = -8; 
int oldDrawX = -1;
int oldDrawY = -1;

unsigned long lastFrameTime = 0;

// ------------------------------------------------------------------------------
// I2C BIT-BANGING & OLED DRIVERS 
// ------------------------------------------------------------------------------
void i2c_write(uint8_t data) {
  for (uint8_t i = 0; i < 8; i++) {
    if (data & 0x80) PORTB |= (1 << OLED_SDA); else PORTB &= ~(1 << OLED_SDA);
    PORTB |= (1 << OLED_SCL); PORTB &= ~(1 << OLED_SCL);
    data <<= 1;
  }
  PORTB |= (1 << OLED_SCL); PORTB &= ~(1 << OLED_SCL);
}

void oled_command(uint8_t cmd) {
  PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
  PORTB &= ~(1 << OLED_SDA); PORTB &= ~(1 << OLED_SCL);
  i2c_write(0x78); i2c_write(0x00); i2c_write(cmd);
  PORTB &= ~(1 << OLED_SDA);
  PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
}

void oled_set_cursor(uint8_t x, uint8_t y) {
  oled_command(0xB0 + y);                 
  oled_command(0x00 + (x & 0x0F));        
  oled_command(0x10 + ((x >> 4) & 0x0F)); 
}

void oled_clear() {
  for (uint8_t y = 0; y < 8; y++) {
    oled_set_cursor(0, y);
    PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
    PORTB &= ~(1 << OLED_SDA); PORTB &= ~(1 << OLED_SCL);
    i2c_write(0x78); i2c_write(0x40);
    for (uint8_t x = 0; x < 128; x++) i2c_write(0x00);
    PORTB &= ~(1 << OLED_SDA);
    PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
  }
}

// ------------------------------------------------------------------------------
// GRAPHICS & SOUND FUNCTIONS
// ------------------------------------------------------------------------------
void drawChar(uint8_t x, uint8_t page, char c) {
  uint8_t idx = 0;
  bool isSpace = false;
  if (c >= '0' && c <= '9') idx = c - '0' + 1;
  else if (c >= 'A' && c <= 'Z') idx = c - 'A' + 11;
  else if (c == '\'') idx = 37; else if (c == '@') idx = 38;
  else if (c == ':') idx = 39; else if (c == '.') idx = 40;
  else if (c == ' ') { isSpace = true; }
  
  oled_set_cursor(x, page);
  PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
  PORTB &= ~(1 << OLED_SDA); PORTB &= ~(1 << OLED_SCL);
  i2c_write(0x78); i2c_write(0x40);
  
  if (isSpace) {
    for(uint8_t i=0; i<6; i++) i2c_write(0x00);
  } else {
    for(uint8_t i=0; i<5; i++) i2c_write(pgm_read_byte(&font5x7[idx * 5 + i]));
    i2c_write(0x00); 
  }
  PORTB &= ~(1 << OLED_SDA); PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
}

void drawString(uint8_t x, uint8_t page, const char* str) {
  while (*str) { drawChar(x, page, *str++); x += 6; }
}

void drawInt(uint8_t x, uint8_t page, int val) {
  char str[6]; itoa(val, str, 10); drawString(x, page, str);
}

void drawUI() {
  drawInt(0, 0, score);
  drawString(95, 0, "L:"); drawInt(110, 0, lives);
}

void drawSprite(int x, uint8_t page, const uint8_t* sprite, uint8_t width, bool eraseSides = false) {
  if (x < 0 || x > 127) return; 
  int startX = x;
  if (eraseSides) { if (startX >= 2) startX -= 2; else startX = 0; }
  
  oled_set_cursor(startX, page);
  PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
  PORTB &= ~(1 << OLED_SDA); PORTB &= ~(1 << OLED_SCL);
  i2c_write(0x78); i2c_write(0x40);
  
  if (eraseSides) { i2c_write(0); i2c_write(0); }
  if (sprite != NULL) {
    for(uint8_t i=0; i<width; i++) { if (startX + i <= 127) i2c_write(pgm_read_byte(&sprite[i])); }
  } else {
    for(uint8_t i=0; i<width; i++) i2c_write(0x00); 
  }
  if (eraseSides) { i2c_write(0); i2c_write(0); }
  PORTB &= ~(1 << OLED_SDA); PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
}

void drawLargeSprite(uint8_t x, uint8_t startPage, const uint8_t* sprite, uint8_t width, uint8_t pages) {
  for (uint8_t p = 0; p < pages; p++) {
    oled_set_cursor(x, startPage + p);
    PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
    PORTB &= ~(1 << OLED_SDA); PORTB &= ~(1 << OLED_SCL);
    i2c_write(0x78); i2c_write(0x40);
    for(uint8_t i = 0; i < width; i++) i2c_write(pgm_read_byte(&sprite[(p * width) + i]));
    PORTB &= ~(1 << OLED_SDA); PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
  }
}

void renderBall(int x, int y, bool erase) {
  if (x < 0 || x > 124 || y < 0 || y > 60) return;
  uint8_t page1 = y / 8; uint8_t page2 = (y + 3) / 8;
  uint8_t shift1 = y % 8; uint8_t shift2 = 8 - shift1;
  uint8_t mask = 0b00001111; 

  PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
  for(uint8_t p = page1; p <= page2; p++) {
      oled_set_cursor(x, p);
      PORTB &= ~(1 << OLED_SDA); PORTB &= ~(1 << OLED_SCL);
      i2c_write(0x78); i2c_write(0x40);
      
      for(uint8_t i=0; i<4; i++) {
          uint8_t col = mask;
          if (p == page1) col <<= shift1; else col >>= shift2;
          if (erase) i2c_write(0); else i2c_write(col);
      }
      PORTB &= ~(1 << OLED_SDA); PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
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

void playCustomSong(const int* notes, const int* durations, int length) {
  for(int i=0; i<length; i++) {
    int note = pgm_read_word(&notes[i]);
    int duration = pgm_read_word(&durations[i]);
    if (note > 0) beep(note, duration); else delay(duration);
    delay(20); 
  }
}

bool readLeft() { return (PINB & (1 << BTN_LEFT)); } 
bool readRight() { return (PINB & (1 << BTN_RIGHT)); } 
bool readFire() {
  ADCSRA |= (1 << ADSC); 
  while (ADCSRA & (1 << ADSC)); 
  return (ADC < 950); 
}

// ------------------------------------------------------------------------------
// GAME LOGIC
// ------------------------------------------------------------------------------
void loadLevel(int lvlNum) {
  oled_clear(); drawUI();
  
  int ptrIdx = (lvlNum - 1) % 10; 
  const uint8_t* lvlPtr = (const uint8_t*)pgm_read_word(&levelData[ptrIdx]);
  
  bricksRemaining = 0; modActive = false; paddleMode = 0;
  
  for (int i = 0; i < (BRICK_ROWS * BRICK_COLS); i++) {
    uint8_t bType = pgm_read_byte(&lvlPtr[i]);
    currentLevel[i] = bType;
    if (bType == 1 || bType == 2 || bType == 4) bricksRemaining++;
    
    int row = i / BRICK_COLS; int col = i % BRICK_COLS;
    if (bType > 0) {
      const uint8_t* bSp = (bType==1)?brick_1 : ((bType==2)?brick_2 : ((bType==3)?brick_3 : brick_4));
      drawSprite(4 + (col * 12), row + 1, bSp, 10, false); 
    }
  }
  
  paddleX = 56;
  ballX = (paddleX + 6) * 10; ballY = 510; 
  ballDx = BASE_BALL_SPEED + (level/2); ballDy = -(BASE_BALL_SPEED + (level/2));
  
  oldDrawX = -1; oldDrawY = -1;
  drawSprite(paddleX, 7, custom_paddle, 16, false);
}

void setup() {
  DDRB |= (1 << OLED_SDA) | (1 << OLED_SCL) | (1 << BUZZER);
  DDRB &= ~((1 << BTN_LEFT) | (1 << BTN_RIGHT));
  PORTB &= ~((1 << BTN_LEFT) | (1 << BTN_RIGHT)); 
  
  ADMUX = 0b00000000;  
  ADCSRA = 0b10000011; 

  oled_command(0xAE); oled_command(0x20); oled_command(0x02); 
  oled_command(0xA1); oled_command(0xC8); 
  oled_command(0x8D); oled_command(0x14); 
  oled_command(0xAF); 
  
  EEPROM.get(0, highScore);
  if (highScore == -1 || highScore > 9999) highScore = 0; 
}

void loop() {
  unsigned long currentMillis = millis();

  // STATE 0: HTL LOGO SPLASH
  if (gameState == 0) {
    oled_clear();
    drawLargeSprite(48, 2, htl_logo, 32, 4); 
    drawString(25, 7, "BRICK BREAKER");
    delay(3000); 
    gameState = 1; 
  }

  // STATE 1: CREDITS & HIGH SCORE
  if (gameState == 1) {
    oled_clear();
    drawString(3, 0, "GIRLS' DAY"); drawString(3, 1, "@ HTL KAINDORF");
    drawString(3, 3, "BUILT BY:"); drawString(60, 3, PLAYER_NAME); 
    drawString(3, 4, "ON "); drawString(25, 4, BUILD_DATE);  
    drawString(3, 6, "HIGH SCORE:"); drawInt(75, 6, highScore);
    
    while(!readFire()) { /* Wait */ }
    beep(1000, 100);
    score = 0; lives = STARTING_LIVES; level = 1;
    gameState = 2; 
  }

  // STATE 2: LEVEL START
  if (gameState == 2) {
    loadLevel(level);
    drawString(40, 4, "READY?");
    delay(1000);
    drawString(40, 4, "      "); 
    lastFrameTime = millis();
    gameState = 3;
  }

  // STATE 3: PLAYING
  if (gameState == 3) {
    if (currentMillis - lastFrameTime < (1000 / GAME_FPS)) return; 
    lastFrameTime = currentMillis;

    // --- Flawless Paddle Movement & Boundary Logic ---
    int pW = (paddleMode == 1) ? 24 : 16;
    
    if (readLeft()) {
      paddleX -= PADDLE_SPEED;
      if (paddleX < 0) paddleX = 0;
    }
    if (readRight()) {
      paddleX += PADDLE_SPEED;
      if (paddleX > 128 - pW) paddleX = 128 - pW; // Prevents "sinking" into the wall!
    }
    
    const uint8_t* pSprite = (paddleMode == 1) ? custom_paddle_wide : custom_paddle;
    drawSprite(paddleX, 7, pSprite, pW, true); 

    // --- Mod Timers ---
    if (paddleMode == 1) {
      paddleTimer--;
      if (paddleTimer <= 0) {
        paddleMode = 0;
        drawSprite(paddleX, 7, NULL, 24, false); 
        beep(500, 50); 
      }
    }

    if (modActive) {
      modTick++;
      if (modTick > 2) {
        modTick = 0;
        drawSprite(modX, modPage, NULL, 8, false); 
        modPage++;
        if (modPage == 7) { 
          if (modX + 8 >= paddleX && modX <= paddleX + pW) { 
            paddleMode = 1; paddleTimer = 300; 
            if (paddleX > 128 - 24) paddleX = 128 - 24; 
            beep(1000, 30); beep(1500, 30);
            drawSprite(paddleX - 4, 7, NULL, 32, false); 
          }
          modActive = false;
        } else if (modPage > 7) {
          modActive = false;
        } else {
          drawSprite(modX, modPage, fx_powerup, 8, false); 
        }
      }
    }

    // --- AABB Physics Engine ---
    ballX += ballDx; ballY += ballDy;
    
    int drawX = ballX / 10; int drawY = ballY / 10;
    int ballRight = drawX + 3; int ballBottom = drawY + 3;

    // Wall Bounces (Shortened beeps prevent audio lag!)
    if (drawX <= 0) { ballDx = abs(ballDx); ballX = 10; beep(600, 3); }
    if (ballRight >= 127) { ballDx = -abs(ballDx); ballX = 1230; beep(600, 3); }
    if (drawY <= 9) { ballDy = abs(ballDy); ballY = 100; beep(600, 3); }

    // Floor (Death)
    if (drawY >= 60) {
      lives--;
      beep(150, 300); delay(500);
      renderBall(oldDrawX, oldDrawY, true); 
      drawUI(); paddleMode = 0; 
      
      if (lives <= 0) gameState = 4;
      else {
        drawSprite(paddleX, 7, NULL, 24, false); 
        paddleX = 56;
        ballX = (paddleX + 6) * 10; ballY = 510; 
        ballDx = BASE_BALL_SPEED + (level/2); ballDy = -(BASE_BALL_SPEED + (level/2));
        oldDrawX = -1; oldDrawY = -1;
        delay(500);
      }
    }

    // Paddle Bounce
    if (ballBottom >= 52 && drawY <= 54 && ballDy > 0) {
      if (ballRight >= paddleX && drawX <= paddleX + pW) {
        ballDy = -ballDy; ballY = 480;
        int hitPos = (drawX + 2) - (paddleX + pW / 2);
        ballDx = hitPos * 1; 
        if (ballDx > 12) ballDx = 12; if (ballDx < -12) ballDx = -12;
        if (ballDx == 0) ballDx = (random(2) == 0) ? 4 : -4; 
        beep(800, 8); // Shortened beep!
      }
    }

    // AABB Brick Bounce
    bool bouncedX = false;
    bool bouncedY = false;

    if (ballBottom >= 8 && drawY <= 39) {
      for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
          int idx = row * BRICK_COLS + col;
          uint8_t bType = currentLevel[idx];

          if (bType > 0) {
            int brickLeft = 4 + (col * 12);
            int brickRight = brickLeft + 9;
            int brickTop = 8 + (row * 8);
            int brickBottom = brickTop + 7;

            if (ballRight >= brickLeft && drawX <= brickRight &&
                ballBottom >= brickTop && drawY <= brickBottom) {

                int oldX = (oldDrawX == -1) ? drawX : oldDrawX;
                int oldY = (oldDrawY == -1) ? drawY : oldDrawY;
                
                if (oldX + 3 < brickLeft || oldX > brickRight) {
                  if (!bouncedX) { ballDx = -ballDx; bouncedX = true; }
                }
                if (oldY + 3 < brickTop || oldY > brickBottom) {
                  if (!bouncedY) { ballDy = -ballDy; bouncedY = true; }
                }
                if (!bouncedX && !bouncedY) {
                  ballDx = -ballDx; ballDy = -ballDy;
                  bouncedX = true; bouncedY = true;
                }

                ballX += ballDx; ballY += ballDy;
                drawX = ballX / 10; drawY = ballY / 10;
                
                beep(1500, 8); // Shortened beep!

                if (bType == 1 || bType == 4) {
                  currentLevel[idx] = 0; bricksRemaining--; score += BRICK_POINTS;
                  drawSprite(brickLeft - 2, row + 1, NULL, 14, false); 
                  drawUI();

                  if (bType == 4 && !modActive) {
                    modActive = true; modX = brickLeft + 1; modPage = row + 1; modTick = 0;
                  }
                } else if (bType == 2) {
                  currentLevel[idx] = 1; score += (BRICK_POINTS / 2);
                  drawSprite(brickLeft, row + 1, brick_1, 10, false);
                  drawUI();
                }

                // Repair adjacent bricks
                if (col > 0 && currentLevel[idx - 1] > 0) {
                   uint8_t t = currentLevel[idx - 1];
                   const uint8_t* sp = (t==1)?brick_1 : ((t==2)?brick_2 : ((t==3)?brick_3 : brick_4));
                   drawSprite(4 + ((col-1)*12), row + 1, sp, 10, false);
                }
                if (col < BRICK_COLS - 1 && currentLevel[idx + 1] > 0) {
                   uint8_t t = currentLevel[idx + 1];
                   const uint8_t* sp = (t==1)?brick_1 : ((t==2)?brick_2 : ((t==3)?brick_3 : brick_4));
                   drawSprite(4 + ((col+1)*12), row + 1, sp, 10, false);
                }

                if (bricksRemaining == 0) {
                  playCustomSong(levelUpMelody, levelUpTempo, 4);
                  level++; gameState = 2; return;
                }
            }
          }
        }
      }
    }

    if (gameState == 3) {
      if (oldDrawX != -1) renderBall(oldDrawX, oldDrawY, true); 
      renderBall(drawX, drawY, false); 
      oldDrawX = drawX; oldDrawY = drawY;
    }
  }

  // STATE 4: GAME OVER
  if (gameState == 4) {
    if (score > highScore) { highScore = score; EEPROM.put(0, highScore); }

    oled_clear();
    drawString(35, 2, "GAME OVER");
    drawString(30, 4, "SCORE: "); drawInt(70, 4, score);
    
    playCustomSong(gameOverMelody, gameOverTempo, 4);
    delay(1000);
    drawString(20, 6, "PRESS FIRE");
    
    while(!readFire()) { /* Wait */ }
    delay(300); gameState = 1; 
  }
}