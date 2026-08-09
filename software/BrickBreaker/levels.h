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