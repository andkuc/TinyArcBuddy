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
// elements.h - THE FULL PERIODIC TABLE DATABASE
// ==============================================================================
// 118 Elements perfectly packed into the ATtiny85's memory using Look-up Tables!

#ifndef ELEMENTS_H
#define ELEMENTS_H
#include <avr/pgmspace.h>

#define ELEMENT_COUNT 118

// --- THE DICTIONARY (Lookup Table) ---
// This compresses thousands of characters down to single bytes!
const char cat_0[] PROGMEM = "Nonmetal";
const char cat_1[] PROGMEM = "Noble Gas";
const char cat_2[] PROGMEM = "Alkali Metal";
const char cat_3[] PROGMEM = "Alk Earth Metal";
const char cat_4[] PROGMEM = "Metalloid";
const char cat_5[] PROGMEM = "Halogen";
const char cat_6[] PROGMEM = "Post-Tran Metal";
const char cat_7[] PROGMEM = "Transition Met.";
const char cat_8[] PROGMEM = "Lanthanide";
const char cat_9[] PROGMEM = "Actinide";
const char cat_10[] PROGMEM = "Unknown";

const char* const categoryDict[] PROGMEM = {
  cat_0, cat_1, cat_2, cat_3, cat_4, cat_5, cat_6, cat_7, cat_8, cat_9, cat_10
};

// ==============================================================================
// THE DATA ARRAYS (Grouped in rows of 10)
// ==============================================================================

// 1. The Symbols (Max 2 letters + Null terminator)
const char syms[ELEMENT_COUNT][3] PROGMEM = {
  "H",  "He", "Li", "Be", "B",  "C",  "N",  "O",  "F",  "Ne",
  "Na", "Mg", "Al", "Si", "P",  "S",  "Cl", "Ar", "K",  "Ca",
  "Sc", "Ti", "V",  "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn",
  "Ga", "Ge", "As", "Se", "Br", "Kr", "Rb", "Sr", "Y",  "Zr",
  "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn",
  "Sb", "Te", "I",  "Xe", "Cs", "Ba", "La", "Ce", "Pr", "Nd",
  "Pm", "Sm", "Eu", "Gd", "Tb", "Dy", "Ho", "Er", "Tm", "Yb",
  "Lu", "Hf", "Ta", "W",  "Re", "Os", "Ir", "Pt", "Au", "Hg",
  "Tl", "Pb", "Bi", "Po", "At", "Rn", "Fr", "Ra", "Ac", "Th",
  "Pa", "U",  "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm",
  "Md", "No", "Lr", "Rf", "Db", "Sg", "Bh", "Hs", "Mt", "Ds",
  "Rg", "Cn", "Nh", "Fl", "Mc", "Lv", "Ts", "Og"
};

// 2. The Full Names (Max 14 letters)
const char names[ELEMENT_COUNT][15] PROGMEM = {
  "Hydrogen", "Helium", "Lithium", "Beryllium", "Boron", "Carbon", "Nitrogen", "Oxygen", "Fluorine", "Neon",
  "Sodium", "Magnesium", "Aluminum", "Silicon", "Phosphorus", "Sulfur", "Chlorine", "Argon", "Potassium", "Calcium",
  "Scandium", "Titanium", "Vanadium", "Chromium", "Manganese", "Iron", "Cobalt", "Nickel", "Copper", "Zinc",
  "Gallium", "Germanium", "Arsenic", "Selenium", "Bromine", "Krypton", "Rubidium", "Strontium", "Yttrium", "Zirconium",
  "Niobium", "Molybdenum", "Technetium", "Ruthenium", "Rhodium", "Palladium", "Silver", "Cadmium", "Indium", "Tin",
  "Antimony", "Tellurium", "Iodine", "Xenon", "Cesium", "Barium", "Lanthanum", "Cerium", "Praseodymium", "Neodymium",
  "Promethium", "Samarium", "Europium", "Gadolinium", "Terbium", "Dysprosium", "Holmium", "Erbium", "Thulium", "Ytterbium",
  "Lutetium", "Hafnium", "Tantalum", "Tungsten", "Rhenium", "Osmium", "Iridium", "Platinum", "Gold", "Mercury",
  "Thallium", "Lead", "Bismuth", "Polonium", "Astatine", "Radon", "Francium", "Radium", "Actinium", "Thorium",
  "Protactinium", "Uranium", "Neptunium", "Plutonium", "Americium", "Curium", "Berkelium", "Californium", "Einsteinium", "Fermium",
  "Mendelevium", "Nobelium", "Lawrencium", "Rutherfordium", "Dubnium", "Seaborgium", "Bohrium", "Hassium", "Meitnerium", "Darmstadtium",
  "Roentgenium", "Copernicium", "Nihonium", "Flerovium", "Moscovium", "Livermorium", "Tennessine", "Oganesson"
};

// 3. Category Codes (Matches the Dictionary above!)
const uint8_t catCodes[ELEMENT_COUNT] PROGMEM = {
  0,  1,  2,  3,  4,  0,  0,  0,  5,  1,
  2,  3,  6,  4,  0,  0,  5,  1,  2,  3,
  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
  6,  4,  4,  0,  5,  1,  2,  3,  7,  7,
  7,  7,  7,  7,  7,  7,  7,  7,  6,  6,
  4,  4,  5,  1,  2,  3,  8,  8,  8,  8,
  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
  8,  7,  7,  7,  7,  7,  7,  7,  7,  7,
  6,  6,  6,  6,  5,  1,  2,  3,  9,  9,
  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,
  9,  9,  9,  7,  7,  7,  7,  7,  10, 10,
  10, 7,  10, 6,  10, 6,  10, 10
};

// 4. Atomic Mass (Fixed-point: divide by 1000). 
// Note: Uses uint32_t so heavy elements don't overflow!
const uint32_t masses[ELEMENT_COUNT] PROGMEM = {
  1008,   4002,   6940,   9012,   10810,  12011,  14007,  15999,  18998,  20180,
  22990,  24305,  26982,  28085,  30974,  32060,  35450,  39948,  39098,  40078,
  44956,  47867,  50942,  51996,  54938,  55845,  58933,  58693,  63546,  65380,
  69723,  72630,  74922,  78960,  79904,  83798,  85468,  87620,  88906,  91224,
  92906,  95950,  98000,  101070, 102906, 106420, 107868, 112411, 114818, 118710,
  121760, 127600, 126904, 131293, 132905, 137327, 138905, 140116, 140907, 144242,
  145000, 150360, 151964, 157250, 158925, 162500, 164930, 167259, 168934, 173045,
  174967, 178490, 180948, 183840, 186207, 190230, 192217, 195084, 196967, 200592,
  204380, 207200, 208980, 209000, 210000, 222000, 223000, 226000, 227000, 232038,
  231036, 238029, 237000, 244000, 243000, 247000, 247000, 251000, 252000, 257000,
  258000, 259000, 266000, 267000, 268000, 269000, 270000, 269000, 278000, 281000,
  282000, 285000, 286000, 289000, 290000, 293000, 294000, 294000
};

// 5. Melting Point (In Kelvin, to avoid negative numbers! 0 = Unknown/Superheavy)
const int meltingK[ELEMENT_COUNT] PROGMEM = {
  14,   1,    454,  1560, 2349, 3800, 63,   55,   53,   24,
  371,  923,  933,  1687, 317,  388,  172,  84,   337,  1115,
  1814, 1941, 2183, 2180, 1519, 1811, 1768, 1728, 1358, 693,
  303,  1211, 1090, 494,  266,  116,  312,  1050, 1799, 2128,
  2750, 2896, 2430, 2607, 2237, 1828, 1235, 594,  430,  505,
  904,  723,  387,  161,  302,  1000, 1193, 1068, 1208, 1297,
  1315, 1345, 1099, 1585, 1629, 1680, 1734, 1802, 1818, 1097,
  1925, 2506, 3290, 3695, 3459, 3306, 2719, 2041, 1337, 234,
  577,  601,  544,  527,  575,  202,  300,  973,  1323, 2023,
  1841, 1405, 910,  912,  1449, 1613, 1259, 1173, 1133, 1800,
  1100, 1100, 1900, 0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0
};

// 6. Year of Discovery (0 = Ancient/Antiquity)
const int discoverYear[ELEMENT_COUNT] PROGMEM = {
  1766, 1895, 1817, 1798, 1808, 0,    1772, 1774, 1886, 1898,
  1807, 1755, 1825, 1824, 1669, 0,    1774, 1894, 1807, 1808,
  1879, 1791, 1801, 1797, 1774, 0,    1735, 1751, 0,    0,
  1875, 1886, 1250, 1817, 1826, 1898, 1861, 1790, 1794, 1789,
  1801, 1778, 1937, 1844, 1803, 1803, 0,    1817, 1863, 0,
  0,    1782, 1811, 1898, 1860, 1808, 1839, 1803, 1885, 1885,
  1945, 1879, 1896, 1880, 1843, 1886, 1878, 1843, 1879, 1878,
  1907, 1923, 1802, 1783, 1925, 1803, 1803, 1735, 0,    0,
  1861, 0,    1753, 1898, 1940, 1899, 1939, 1898, 1899, 1829,
  1913, 1789, 1940, 1940, 1944, 1944, 1949, 1950, 1952, 1952,
  1955, 1958, 1961, 1969, 1970, 1974, 1981, 1984, 1982, 1994,
  1994, 1996, 2003, 1999, 2003, 2000, 2010, 2002
};

#endif