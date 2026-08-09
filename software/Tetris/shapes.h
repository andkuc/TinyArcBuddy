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