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
// formulas.h - THE MATHEMATICS DICTIONARY (WITH UNIT GROUPS)
// ==============================================================================

#ifndef FORMULAS_H
#define FORMULAS_H
#include <avr/pgmspace.h>

// --- 1. Main Categories ---
const char cat0[] PROGMEM = "1. GEOMETRY";
const char cat1[] PROGMEM = "2. PHYSICS";
const char cat2[] PROGMEM = "3. ALGEBRA";
const char* const catStrings[] PROGMEM = {cat0, cat1, cat2};

// --- 2. The 14 Tools ---
const char t0[] PROGMEM = "CYLINDER VOL.";
const char t1[] PROGMEM = "TRIANGLE HYP.";
const char t2[] PROGMEM = "CIRCLE AREA";
const char t3[] PROGMEM = "SPHERE VOL.";
const char t4[] PROGMEM = "RECT. AREA";

const char t5[] PROGMEM = "SPEED (v=s/t)";
const char t6[] PROGMEM = "OHM'S LAW (U=RI)";
const char t7[] PROGMEM = "DENSITY(p=m/V)";
const char t8[] PROGMEM = "WORK (W=F*s)";
const char t9[] PROGMEM = "KIN. ENERGY";

const char t10[] PROGMEM = "2x2 EQUATIONS";
const char t11[] PROGMEM = "PERCENT OF Y";
const char t12[] PROGMEM = "FRACTION->DEC";
const char t13[] PROGMEM = "QUADRATIC EQ.";

const char* const toolStrings[] PROGMEM = {
  t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13
};

const uint8_t toolCats[] PROGMEM = { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2 };
const uint8_t toolInputCounts[] PROGMEM = { 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 6, 2, 2, 3 };

// --- 3. Unit Groups for Pre-Selection ---
const char u_mm[] PROGMEM = "mm";
const char u_cm[] PROGMEM = "cm";
const char u_m[] PROGMEM  = "m";

const char u_ms[] PROGMEM = "m/s";
const char u_kmh[] PROGMEM = "km/h";

const char u_kgm3[] PROGMEM = "kg/m3";
const char u_gcm3[] PROGMEM = "g/cm3";

const char* const group_geo[] PROGMEM = {u_mm, u_cm, u_m};
const char* const group_spd[] PROGMEM = {u_ms, u_kmh};
const char* const group_den[] PROGMEM = {u_kgm3, u_gcm3};

// Tool to Unit Group Mapping: 0=Geo, 1=Speed, 2=Density, 3=Skip Selection(Fixed/None)
const uint8_t toolUnitGroup[] PROGMEM = {
  0, 0, 0, 0, 0,  // Geometry
  1, 3, 2, 3, 3,  // Physics
  3, 3, 3, 3      // Algebra
};
const uint8_t groupSizes[] PROGMEM = {3, 2, 2, 0};
const char* const* const unitGroups[] PROGMEM = {group_geo, group_spd, group_den};


// --- 4. The Input Prompts ---
const char p_r[] PROGMEM  = "Radius r=";
const char p_h[] PROGMEM  = "Height h=";
const char p_a[] PROGMEM  = "Side a=";
const char p_b[] PROGMEM  = "Side b=";
const char p_s[] PROGMEM  = "Dist. s=";
const char p_t[] PROGMEM  = "Time t=";
const char p_R[] PROGMEM  = "Res. Ohm):";
const char p_I[] PROGMEM  = "Curr. I [A]:";
const char p_m[] PROGMEM  = "Mass m=";
const char p_V[] PROGMEM  = "Vol. V=";
const char p_F[] PROGMEM  = "Force F [N]:";
const char p_w[] PROGMEM  = "Dist. s [m]:";
const char p_kg[] PROGMEM = "Mass m [kg]:";
const char p_v[] PROGMEM  = "Spd v [m/s]:";
const char p_pct[] PROGMEM = "Percent X%:";
const char p_Y[] PROGMEM   = "Of value Y:";
const char p_num[] PROGMEM = "Numerator:";
const char p_den[] PROGMEM = "Denominator:";
const char p_qa[] PROGMEM = "Eq: a =";
const char p_qb[] PROGMEM = "Eq: b =";
const char p_qc[] PROGMEM = "Eq: c =";

// Link prompts to specific tools
const char* const prompts_0[] PROGMEM = {p_r, p_h}; 
const char* const prompts_1[] PROGMEM = {p_a, p_b}; 
const char* const prompts_2[] PROGMEM = {p_r};      
const char* const prompts_3[] PROGMEM = {p_r};      
const char* const prompts_4[] PROGMEM = {p_a, p_b}; 
const char* const prompts_5[] PROGMEM = {p_s, p_t}; 
const char* const prompts_6[] PROGMEM = {p_R, p_I}; 
const char* const prompts_7[] PROGMEM = {p_m, p_V}; 
const char* const prompts_8[] PROGMEM = {p_F, p_w}; 
const char* const prompts_9[] PROGMEM = {p_kg, p_v}; 

// Tool 10 (2x2) uses a custom drawn UI, but we provide placeholders to prevent crashes
const char* const prompts_10[] PROGMEM = {p_a, p_b, p_h, p_a, p_b, p_h}; 
const char* const prompts_11[] PROGMEM = {p_pct, p_Y}; 
const char* const prompts_12[] PROGMEM = {p_num, p_den}; 
const char* const prompts_13[] PROGMEM = {p_qa, p_qb, p_qc}; 

const char* const* const toolPrompts[] PROGMEM = {
  prompts_0, prompts_1, prompts_2, prompts_3, prompts_4,
  prompts_5, prompts_6, prompts_7, prompts_8, prompts_9,
  prompts_10, prompts_11, prompts_12, prompts_13
};

#endif