#define DEBUG

#include <Arduino.h>
#include <stdlib.h>
#include "errorhandling.h"
#include "threading.h"
#include "strtools.h"

// #####################################################
// ### Useful Functions
uint16_t fstr_len(const __FlashStringHelper *ifsh){
  const char PROGMEM *p = (const char PROGMEM *)ifsh;
  size_t n = 0;
  while (1) {
    unsigned char c = pgm_read_byte(p++);
    if (c == 0) break;
    n++;
  }
  return n;
}

uint8_t cmp_str_flash(char *str, const __FlashStringHelper *flsh){
  const char PROGMEM *p = (const char PROGMEM *)flsh;
  size_t n = 0;
  char c = 1;
  while (c != 0) {
    c = pgm_read_byte(p++);
    if (c != *(str++)) return false;
    //Serial.write(c);
    //Serial.write(*(str-1));
    //Serial.println();
  }
  return true;
}

uint8_t cmp_flash_flash(const __FlashStringHelper *flsh1, const __FlashStringHelper *flsh2){
 unsigned char c1 = 1, c2;
  const char PROGMEM *p1 = (const char PROGMEM *)flsh1;
  const char PROGMEM *p2 = (const char PROGMEM *)flsh2;
  while (c1 != 0) {
    c1 = pgm_read_byte(p1++);
    c2 = pgm_read_byte(p2++);
    if (c1 != c2) return false;
  }
  return true;
}

// cmp_str_el(string, len, element) -- compares the string with length len to the element
uint8_t __cmp_str_elptr(char *name, uint16_t name_len, TH_element *el){
  debug(String("cmp:") + name);
  debug(el->name);
  debug(String(name_len) + String(" ") + String(el->name_len));
  if(el->name_len != name_len) return false;
  debug("len same");
  if(cmp_str_flash(name, el->name)) return true; 
  else return false;
}

//compare flash helper with element pointer
uint8_t cmp_flhp_elptr(const __FlashStringHelper *flph, uint8_t len, TH_element *el){
  debug("cmp flhp");
  debug(flph);
  debug(el->name);
  if(el->name_len != len) return false;
  if(!cmp_flash_flash(flph, el->name)) return false;
  debug("equal");
  return true;
}

