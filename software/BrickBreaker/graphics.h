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
// graphics.h - THE ART STUDIO
// ==============================================================================
#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <avr/pgmspace.h>

// --- HTL Kaindorf Logo (32x32 Pixels) ---
const uint8_t htl_logo[] PROGMEM = {
    0x00, 0x00, 0xF0, 0xF8, 0x0C, 0x0C, 0xCC, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C,
    0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0x4C, 0xCC, 0x0C, 0x0C, 0xF8, 0xF0, 0x00, 0x00,
    0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x20, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x20, 0x2C, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
    0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x03, 0x02, 0x02, 0xC2, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x82, 0x02, 0x03, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00,
    0x00, 0x00, 0x0F, 0x1F, 0x30, 0x30, 0x30, 0x31, 0x31, 0x37, 0x31, 0x31, 0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30, 0x32, 0x34, 0x30, 0x30, 0x31, 0x30, 0x30, 0x30, 0x1F, 0x0F, 0x00, 0x00
};

// --- Brick Breaker Graphics ---

// The Normal Paddle (16 pixels wide)
const uint8_t custom_paddle[] PROGMEM = { 
  0x7E, 0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 
  0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF, 0x7E 
};

// The WIDE Paddle! (24 pixels wide)
const uint8_t custom_paddle_wide[] PROGMEM = { 
  0x7E, 0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 
  0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
  0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF, 0x7E 
};

// Brick Type 1: Normal 
const uint8_t brick_1[] PROGMEM = { 0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF };

// Brick Type 2: Tough (Checked Pattern)
const uint8_t brick_2[] PROGMEM = { 0xFF, 0xAB, 0xD5, 0xAB, 0xD5, 0xAB, 0xD5, 0xAB, 0xD5, 0xFF };

// Brick Type 3: Metal (Solid box)
const uint8_t brick_3[] PROGMEM = { 0xFF, 0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF };

// Brick Type 4: Mod Brick! (Striped)
const uint8_t brick_4[] PROGMEM = { 0xFF, 0x81, 0xA5, 0x81, 0xA5, 0x81, 0xA5, 0x81, 0x81, 0xFF };

// The Power-Up Pill! (Drops from the Mod Brick)
const uint8_t fx_powerup[] PROGMEM = { 0x3C, 0x7E, 0xE7, 0xC3, 0xC3, 0xE7, 0x7E, 0x3C }; 

#endif