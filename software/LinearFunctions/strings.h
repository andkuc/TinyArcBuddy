// ==============================================================================
// strings.h - UI TEXT FOR THE LINEAR FUNCTION EXPLORER
// ==============================================================================

#ifndef STRINGS_H
#define STRINGS_H
#include <avr/pgmspace.h>

// --- Menus ---
const char m_mode0[] PROGMEM = "1. ALG -> GEOM";
const char m_mode1[] PROGMEM = "2. GEOM -> ALG";
const char* const modeStrings[] PROGMEM = {m_mode0, m_mode1};

const char m_cnt1[] PROGMEM = "1 FUNCTION";
const char m_cnt2[] PROGMEM = "2 FUNCTIONS";
const char* const countStrings[] PROGMEM = {m_cnt1, m_cnt2};

#endif