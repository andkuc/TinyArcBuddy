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
// shapes.h - THE SHAPE STUDIO
// ==============================================================================
// Each shape is a 4x4 grid. We store them as 16-bit Hex values.
// Example: The "I" shape (0x0F00) is:
// 0000 (Row 0)
// 1111 (Row 1)
// 0000 (Row 2)
// 0000 (Row 3)

#ifndef SHAPES_H
#define SHAPES_H
#include <avr/pgmspace.h>

const uint16_t shapes[7][4] PROGMEM = {
  // 0: "I" Shape (Cyan)
  { 0x0F00, 0x2222, 0x00F0, 0x4444 },
  
  // 1: "O" Shape (Yellow)
  { 0x0660, 0x0660, 0x0660, 0x0660 },
  
  // 2: "T" Shape (Purple)
  { 0x0E40, 0x4C40, 0x4E00, 0x4640 },
  
  // 3: "S" Shape (Green)
  { 0x06C0, 0x4620, 0x0360, 0x4620 },
  
  // 4: "Z" Shape (Red)
  { 0x0C60, 0x2640, 0x0630, 0x2640 },
  
  // 5: "J" Shape (Blue)
  { 0x08E0, 0x6440, 0x0E20, 0x44C0 },
  
  // 6: "L" Shape (Orange)
  { 0x02E0, 0x4460, 0x0E80, 0xC440 }
};

#endif