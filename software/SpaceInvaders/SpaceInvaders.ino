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
 * TINY ARC BUDDY - SPACE INVADERS (DELUXE EDITION)
 * ==============================================================================
 */

#include <avr/io.h>
#include <util/delay.h>
#include <EEPROM.h>

#include "font.h"     
#include "graphics.h" 

// ==============================================================================
// ZONE 1: THE GAME RULES (Modify These!)
// ==============================================================================

#define PLAYER_NAME "ANNA"           
#define BUILD_DATE "25.04.2026"      

#define STARTING_LIVES 3             
#define ALIEN_POINTS 10              
#define UFO_POINTS 50                // Bonus points for shooting the UFO!

// --- Custom Events ---
#define POWERUP_CHANCE 5             // 5% chance an alien drops an extra life!
#define UFO_CHANCE 5                 // Chance a UFO appears at the top of the screen

// --- Music Studio (Frequencies in Hz, Tempo in ms) ---
// Older students: Show the girls a Piano Frequency chart! (e.g. 440 = A4)
const int levelUpMelody[] PROGMEM = { 440, 554, 659, 880 }; 
const int levelUpTempo[]  PROGMEM = { 100, 100, 100, 300 };

const int gameOverMelody[] PROGMEM = { 300, 250, 200, 150 };
const int gameOverTempo[]  PROGMEM = { 300, 300, 300, 600 };

// --- Global Game Pacing ---
#define GAME_FPS 30                  
#define SHIP_SPEED 2                 
#define LASER_SPEED 5                
#define ALIEN_BASE_SPEED 5           
#define ALIEN_SLOW_FACTOR 1          
#define BOMB_CHANCE 2                

// --- Difficulty Scaling ---
#define LEVEL_SPEED_BOOST 1          
#define LEVEL_BOMB_BOOST 2           


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
int lives = STARTING_LIVES;
int level = 1;

// --- Player State ---
int shipX = 60; 
int laserX = -1;
int laserPage = -1;
bool isFired = false;

// --- Background & Bunkers ---
int starX[3] = {10, 60, 110};
int starPage[3] = {1, 4, 7};
int starTick = 0;
byte bunkerHP[3] = {3, 3, 3}; // 3 Bunkers, 3 Health each!

// --- Alien & Enemy State ---
#define ALIEN_ROWS 3
#define ALIEN_COLS 5 
byte aliens[ALIEN_ROWS]; 
int alienX = 10;
int alienPage = 2; // Start lower to leave room for UFO!
int alienDir = 2; 
int aliveCount = 0;
bool animFrame = false;

int bombX = -1;
int bombPage = -1;
bool isBombActive = false;

int ufoX = -16;
bool ufoActive = false;
int ufoTick = 0;

int powerX = -1;
int powerPage = -1;
bool powerActive = false;
int powerTick = 0;

// --- Timing (Tick Engine) ---
unsigned long lastFrameTime = 0;
int laserTick = 0;
int alienTick = 0;
int bombTick = 0;
int marchIndex = 0;
const int marchNotes[] = { 200, 150, 100, 50 };


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
  if (c >= '0' && c <= '9') idx = c - '0' + 1;
  else if (c >= 'A' && c <= 'Z') idx = c - 'A' + 11;
  else if (c == '\'') idx = 37;
  else if (c == '@') idx = 38;
  else if (c == ':') idx = 39;
  else if (c == '.') idx = 40;
  else if (c == ' ') { /* Space */ }
  
  oled_set_cursor(x, page);
  PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
  PORTB &= ~(1 << OLED_SDA); PORTB &= ~(1 << OLED_SCL);
  i2c_write(0x78); i2c_write(0x40);
  for(uint8_t i=0; i<5; i++) i2c_write(pgm_read_byte(&font5x7[idx * 5 + i]));
  i2c_write(0x00); 
  PORTB &= ~(1 << OLED_SDA);
  PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
}

void drawString(uint8_t x, uint8_t page, const char* str) {
  while (*str) { drawChar(x, page, *str++); x += 6; }
}

void drawInt(uint8_t x, uint8_t page, int val) {
  char str[6]; itoa(val, str, 10); drawString(x, page, str);
}

void drawUI() {
  drawInt(0, 0, score);
  drawString(95, 0, "L:");
  drawInt(110, 0, lives);
}

void drawSprite(int x, uint8_t page, const uint8_t* sprite, uint8_t width, bool eraseSides = false) {
  if (x < 0 || x > 127) return; // Basic screen bounds safety
  
  int startX = x;
  if (eraseSides) {
    if (startX >= 2) startX -= 2; else startX = 0;
  }
  
  oled_set_cursor(startX, page);
  PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
  PORTB &= ~(1 << OLED_SDA); PORTB &= ~(1 << OLED_SCL);
  i2c_write(0x78); i2c_write(0x40);
  
  if (eraseSides) { i2c_write(0); i2c_write(0); }
  if (sprite != NULL) {
    for(uint8_t i=0; i<width; i++) {
        if (startX + i <= 127) i2c_write(pgm_read_byte(&sprite[i]));
    }
  } else {
    for(uint8_t i=0; i<width; i++) i2c_write(0x00); 
  }
  if (eraseSides) { i2c_write(0); i2c_write(0); }
  
  PORTB &= ~(1 << OLED_SDA);
  PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
}

void drawLargeSprite(uint8_t x, uint8_t startPage, const uint8_t* sprite, uint8_t width, uint8_t pages) {
  for (uint8_t p = 0; p < pages; p++) {
    oled_set_cursor(x, startPage + p);
    PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
    PORTB &= ~(1 << OLED_SDA); PORTB &= ~(1 << OLED_SCL);
    i2c_write(0x78); i2c_write(0x40);
    for(uint8_t i = 0; i < width; i++) i2c_write(pgm_read_byte(&sprite[(p * width) + i]));
    PORTB &= ~(1 << OLED_SDA);
    PORTB |= (1 << OLED_SCL) | (1 << OLED_SDA);
  }
}

// Visual Screen Shake!
void shakeScreen() {
  for(int i=0; i<4; i++) {
    oled_command(0xD3); oled_command(0x02); // Hardware Offset
    delay(30);
    oled_command(0xD3); oled_command(0x00); // Reset
    delay(30);
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

// ------------------------------------------------------------------------------
// INPUT HANDLING
// ------------------------------------------------------------------------------
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
void resetLevel() {
  alienX = 10;
  alienPage = 2; // Leave room for UFO and UI
  alienDir = 2;
  aliveCount = ALIEN_ROWS * ALIEN_COLS;
  for (int i = 0; i < ALIEN_ROWS; i++) aliens[i] = 0b00011111; 
  for (int i = 0; i < 3; i++) bunkerHP[i] = 3;
  
  isFired = false;
  isBombActive = false;
  ufoActive = false;
  powerActive = false;
  oled_clear();
  
  // Draw full Bunkers instantly
  for(int i=0; i<3; i++) drawSprite(15 + i*43, 6, bunker_3, 12, false);
}

void setup() {
  DDRB |= (1 << OLED_SDA) | (1 << OLED_SCL) | (1 << BUZZER);
  DDRB &= ~((1 << BTN_LEFT) | (1 << BTN_RIGHT));
  PORTB &= ~((1 << BTN_LEFT) | (1 << BTN_RIGHT)); 
  
  ADMUX = 0b00000000;  
  ADCSRA = 0b10000011; 

  oled_command(0xAE); 
  oled_command(0x20); oled_command(0x02); 
  oled_command(0xA1); oled_command(0xC8); 
  oled_command(0x8D); oled_command(0x14); 
  oled_command(0xAF); 
  
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
    drawSprite(16, 3, alienA_frame1, 8, false);
    drawSprite(104, 3, alienA_frame2, 8, false);
    drawLargeSprite(48, 2, htl_logo, 32, 4); 
    drawString(25, 7, "SPACE INVADERS");
    
    delay(3000); 
    gameState = 1; 
  }

  // ----------------------------------------------------------------
  // STATE 1: CREDITS & HIGH SCORE
  // ----------------------------------------------------------------
  if (gameState == 1) {
    oled_clear();
    drawString(3, 0, "GIRLS' DAY");
    drawString(3, 1, "@ HTL KAINDORF");
    drawString(3, 3, "BUILT BY:");
    drawString(60, 3, PLAYER_NAME); 
    drawString(3, 4, "ON ");
    drawString(25, 4, BUILD_DATE);  
    drawString(3, 6, "HIGH SCORE:");
    drawInt(75, 6, highScore);
    
    while(!readFire()) { /* Wait */ }
    beep(1000, 100);
    
    score = 0; lives = STARTING_LIVES; level = 1;
    resetLevel();
    gameState = 2; 
    lastFrameTime = millis();
  }

  // ----------------------------------------------------------------
  // STATE 2: PLAYING
  // ----------------------------------------------------------------
  if (gameState == 2) {
    if (currentMillis - lastFrameTime < (1000 / GAME_FPS)) return; 
    lastFrameTime = currentMillis;

    // --- 0. Dynamic Starfield ---
    starTick++;
    if (starTick > 3) {
      starTick = 0;
      for(int i=0; i<3; i++) {
        drawSprite(starX[i], starPage[i], NULL, 1, false); 
        starPage[i]++;
        if (starPage[i] > 7) { starPage[i] = 1; starX[i] = random(0, 128); }
        drawSprite(starX[i], starPage[i], fx_star, 1, false);
      }
    }

    // --- 1. Move & Draw Player ---
    if (readLeft() && shipX > 2) shipX -= SHIP_SPEED;
    if (readRight() && shipX < 116) shipX += SHIP_SPEED; 
    drawSprite(shipX, 7, custom_ship, 8, true); 
    drawUI();

    // --- 2. Player Laser & Collisions ---
    if (readFire() && !isFired) {
      laserX = shipX + 3; 
      laserPage = 6;
      isFired = true;
      laserTick = 0;
      beep(800, 20); 
    }

    if (isFired) {
      laserTick++;
      if (laserTick >= LASER_SPEED) {
        laserTick = 0;
        drawSprite(laserX, laserPage, NULL, 2, false); 
        laserPage--;
        
        // UFO Hit
        if (ufoActive && laserPage == 1 && laserX >= ufoX && laserX <= ufoX + 16) {
            ufoActive = false; isFired = false; score += UFO_POINTS;
            drawSprite(ufoX, 1, fx_explosion, 8, true); beep(2000, 50);
            delay(30); drawSprite(ufoX, 1, NULL, 8, true);
        }

        // Bunker Hit (Player Friendly Fire)
        if (laserPage == 6) {
          for(int i=0; i<3; i++) {
            int bX = 15 + i*43;
            if (bunkerHP[i] > 0 && laserX >= bX && laserX <= bX + 12) {
              bunkerHP[i]--; isFired = false;
              const uint8_t* bSprite = (bunkerHP[i]==2) ? bunker_2 : ((bunkerHP[i]==1) ? bunker_1 : NULL);
              drawSprite(bX, 6, bSprite, 12, false);
            }
          }
        }

        // Alien Hit
        if (isFired && laserPage >= alienPage && laserPage < alienPage + ALIEN_ROWS) {
          int row = laserPage - alienPage;
          for (int c = 0; c < ALIEN_COLS; c++) {
            if (aliens[row] & (1 << c)) {
              int ax = alienX + (c * 12);
              if (laserX >= ax && laserX <= ax + 8) {
                aliens[row] &= ~(1 << c); aliveCount--; isFired = false; 
                score += (ALIEN_POINTS * level); 
                drawSprite(ax, laserPage, fx_explosion, 8, false); beep(2000, 30);
                delay(30); drawSprite(ax, laserPage, NULL, 8, false); 
                
                // Spawn Power-Up!
                if (!powerActive && random(100) < POWERUP_CHANCE) {
                  powerActive = true; powerX = ax + 1; powerPage = laserPage;
                }
              }
            }
          }
        }

        if (isFired) { 
          if (laserPage < 0) isFired = false; 
          else drawSprite(laserX, laserPage, fx_laser, 2, false); 
        }
      }
    }

    // --- 3. Move Aliens ---
    alienTick++;
    int currentAlienSpeed = ALIEN_BASE_SPEED + (aliveCount * ALIEN_SLOW_FACTOR) - (level * LEVEL_SPEED_BOOST);
    if (currentAlienSpeed < 1) currentAlienSpeed = 1; 
    
    if (alienTick >= currentAlienSpeed) {
      alienTick = 0; animFrame = !animFrame; alienX += alienDir;
      
      if (alienX > 60 || alienX < 2) { 
        alienDir = -alienDir; alienX += alienDir; alienPage++; 
        oled_clear(); 
        // Redraw bunkers immediately upon screen wipe
        for(int i=0; i<3; i++) {
          if(bunkerHP[i] > 0) {
            const uint8_t* bSp = (bunkerHP[i]==3) ? bunker_3 : ((bunkerHP[i]==2) ? bunker_2 : bunker_1);
            drawSprite(15 + i*43, 6, bSp, 12, false);
          }
        }
        if (alienPage + ALIEN_ROWS > 7) gameState = 3; 
      }

      for (int r = 0; r < ALIEN_ROWS; r++) {
        const uint8_t* sprite = (r == 0) ? (animFrame ? alienA_frame1 : alienA_frame2) : (animFrame ? alienB_frame1 : alienB_frame2);
        for (int c = 0; c < ALIEN_COLS; c++) {
          if (aliens[r] & (1 << c)) drawSprite(alienX + (c * 12), alienPage + r, sprite, 8, true);
        }
      }

      beep(marchNotes[marchIndex], 15);
      marchIndex = (marchIndex + 1) % 4;
      
      // Spawn Bomb
      int currentBombChance = BOMB_CHANCE + (level * LEVEL_BOMB_BOOST);
      if (currentBombChance > 50) currentBombChance = 50; 
      if (!isBombActive && random(100) < currentBombChance) {
        int randCol = random(ALIEN_COLS);
        for(int r = ALIEN_ROWS - 1; r >= 0; r--) {
          if(aliens[r] & (1 << randCol)) {
            bombX = alienX + (randCol * 12) + 3;
            bombPage = alienPage + r + 1;
            isBombActive = true; bombTick = 0;
            break; 
          }
        }
      }

      if (aliveCount == 0) { 
        level++;
        playCustomSong(levelUpMelody, levelUpTempo, 4);
        oled_clear(); drawString(35, 3, "LEVEL "); drawInt(75, 3, level); delay(1500); 
        resetLevel();
      }
    }

    // --- 4. Move Enemy Bombs ---
    if (isBombActive) {
      bombTick++;
      if (bombTick >= (LASER_SPEED * 2)) {
        bombTick = 0;
        drawSprite(bombX, bombPage, NULL, 3, false); 
        bombPage++;

        // Bunker Hit
        if (bombPage == 6) {
          for(int i=0; i<3; i++) {
            int bX = 15 + i*43;
            if (bunkerHP[i] > 0 && bombX >= bX && bombX <= bX + 12) {
              bunkerHP[i]--; isBombActive = false;
              const uint8_t* bSprite = (bunkerHP[i]==2) ? bunker_2 : ((bunkerHP[i]==1) ? bunker_1 : NULL);
              drawSprite(bX, 6, bSprite, 12, false);
            }
          }
        }

        // Ship Hit
        if (isBombActive && bombPage == 7 && bombX >= shipX && bombX <= shipX + 8) {
          drawSprite(shipX, 7, fx_explosion, 8, true);
          shakeScreen(); // Violent Hardware Shake!
          lives--; isBombActive = false;
          if (lives <= 0) gameState = 3;
        } else if (bombPage > 7) {
          isBombActive = false;
        } else if (isBombActive) {
          drawSprite(bombX, bombPage, fx_bomb, 3, false);
        }
      }
    }

    // --- 5. Mystery UFO ---
    if (!ufoActive && random(1000) < UFO_CHANCE) {
      ufoActive = true; ufoX = 127;
      beep(1500, 30); // UFO arrival ping
    }
    if (ufoActive) {
      ufoTick++;
      if (ufoTick > 1) {
        ufoTick = 0; ufoX -= 2;
        if (ufoX < 0) {
          ufoActive = false; drawSprite(0, 1, NULL, 16, true);
        } else {
          drawSprite(ufoX, 1, ufo_sprite, 16, true);
        }
      }
    }

    // --- 6. Power-Ups ---
    if (powerActive) {
      powerTick++;
      if (powerTick > 3) {
        powerTick = 0;
        drawSprite(powerX, powerPage, NULL, 7, false);
        powerPage++;
        if (powerPage == 7 && powerX + 7 >= shipX && powerX <= shipX + 8) {
          lives++; powerActive = false;
          beep(1000, 100); beep(1500, 100); // Life gained!
        } else if (powerPage > 7) {
          powerActive = false;
        } else {
          drawSprite(powerX, powerPage, fx_heart, 7, false);
        }
      }
    }
  }

  // ----------------------------------------------------------------
  // STATE 3: GAME OVER
  // ----------------------------------------------------------------
  if (gameState == 3) {
    if (score > highScore) { highScore = score; EEPROM.put(0, highScore); }

    oled_clear();
    drawString(35, 2, "GAME OVER");
    drawString(30, 4, "SCORE: "); drawInt(70, 4, score);
    
    playCustomSong(gameOverMelody, gameOverTempo, 4);
    delay(1000);
    drawString(20, 6, "PRESS FIRE");
    
    while(!readFire()) { /* Wait */ }
    delay(300); 
    gameState = 1; 
  }
}