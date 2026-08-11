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

// ==============================================================================
// levels.h - THE LEVEL DESIGNER
// ==============================================================================
// Draw your levels here! The grid is 10 bricks wide and 4 bricks tall.
// 0 = Empty | 1 = Normal | 2 = Tough | 3 = Metal | 4 = Power-Up Brick!

#ifndef LEVELS_H
#define LEVELS_H
#include <avr/pgmspace.h>

#define BRICK_ROWS 4
#define BRICK_COLS 10

// Level 1: A simple starting block (Now with Power-Ups!)
const uint8_t level1[] PROGMEM = {
  1, 1, 1, 1, 4, 4, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Level 2: Checkerboard
const uint8_t level2[] PROGMEM = {
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
  1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
  0, 1, 0, 1, 0, 1, 0, 1, 0, 1
};

// Level 3: The Smiley Face (Nose drops a power-up!)
const uint8_t level3[] PROGMEM = {
  0, 0, 1, 1, 0, 0, 1, 1, 0, 0,
  0, 0, 1, 1, 0, 0, 1, 1, 0, 0,
  1, 0, 0, 0, 4, 4, 0, 0, 0, 1,
  0, 1, 1, 1, 1, 1, 1, 1, 1, 0
};

// Level 4: Introducing Tough Bricks (2)
const uint8_t level4[] PROGMEM = {
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Level 5: The Fortress
const uint8_t level5[] PROGMEM = {
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 0, 0, 0, 0, 0, 0, 0, 0, 2,
  2, 0, 1, 1, 4, 4, 1, 1, 0, 2,
  2, 0, 1, 1, 1, 1, 1, 1, 0, 2
};

// Level 6: Introducing Metal (3)
const uint8_t level6[] PROGMEM = {
  1, 1, 3, 1, 1, 1, 1, 3, 1, 1,
  1, 1, 3, 1, 1, 1, 1, 3, 1, 1,
  1, 1, 3, 1, 1, 1, 1, 3, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

// Level 7: Space Invader Shape
const uint8_t level7[] PROGMEM = {
  0, 2, 0, 0, 2, 2, 0, 0, 2, 0,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 0, 2, 2, 2, 2, 2, 2, 0, 2,
  2, 0, 0, 0, 4, 4, 0, 0, 0, 2
};

// Level 8: Zig Zag
const uint8_t level8[] PROGMEM = {
  3, 1, 1, 0, 0, 0, 0, 1, 1, 3,
  0, 3, 1, 1, 0, 0, 1, 1, 3, 0,
  0, 0, 3, 1, 1, 1, 1, 3, 0, 0,
  0, 0, 0, 3, 2, 2, 3, 0, 0, 0
};

// Level 9: The Cage
const uint8_t level9[] PROGMEM = {
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
  3, 2, 2, 2, 2, 2, 2, 2, 2, 3,
  3, 2, 1, 1, 4, 4, 1, 1, 2, 3,
  3, 0, 0, 0, 0, 0, 0, 0, 0, 3
};

// Level 10: The Final Challenge
const uint8_t level10[] PROGMEM = {
  3, 2, 3, 2, 3, 3, 2, 3, 2, 3,
  2, 3, 2, 3, 2, 2, 3, 2, 3, 2,
  3, 2, 3, 2, 3, 3, 2, 3, 2, 3,
  2, 1, 1, 4, 4, 4, 4, 1, 1, 2
};

// Array of pointers to easily load the levels
const uint8_t* const levelData[10] PROGMEM = {
  level1, level2, level3, level4, level5, 
  level6, level7, level8, level9, level10
};

#endif