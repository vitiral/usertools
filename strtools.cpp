/* Copyright (c) 2014, Garrett Berg <garrett@cloudformdesign.com>, cloudformdesign.com
 * This library is released under the FreeBSD License, if you need
 * a copy go to: http://www.freebsd.org/copyright/freebsd-license.html
 * 
 * SUMMARY:
 * String tools, mostly to help with F() strings
 */

#include "usertools.h"

#include <Arduino.h>
#include <stdlib.h>
#include "errorhandling.h"
#include "threading.h"
#include "strtools.h"


// #####################################################
// ### Useful Functions
uint16_t flash_len(const __FlashStringHelper *ifsh){
  const char PROGMEM *p = (const char PROGMEM *)ifsh;
  size_t n = 0;
  while (1) {
    unsigned char c = pgm_read_byte(p++);
    if (c == 0) break;
    n++;
  }
  return n;
}

void flash_to_str(const __FlashStringHelper *flsh, char *str){
  const char PROGMEM *p = (const char PROGMEM *)flsh;
  size_t n = 0;
  char c = 1;
  while (c != 0) {
    c = pgm_read_byte(p++);
    str[n] = c;
    n++;
  }
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

uint8_t iswhite(char c){
  switch(c){
    case 0x0A:
    case 0x0D:
    case ' ':
    case '\t':
      return true;
  }
  return false;
}

char *pass_ws(char *c){
  while(true) {
    if(iswhite(*c)){
      c++;
    }
    else{
      return c;
    }
  }
}

char *strip(char *c, uint16_t end){
  uint16_t i = 1;
  assert(c[end] == 0);
  while(true){
    if(iswhite(c[end-i])){
      c[end-i] = 0;
    }
    else{
      break;
    }
    i++;
  }
  c = pass_ws(c);
  return c;
error:
  return NULL;
}

char *get_word_end(char *c){
  c = pass_ws(c);
  while(true){
    if(iswhite(*c) or *c == 0) return c;
    c++;
  }
}


// This function works with the macro get_word to modify the input string
// as you run it.
//char *word = get_word(c); // get first word
//char *word2 = get_word(c); // get next word
char *_get_word(char **c){
  // AFFECTS INPUT
  *c = pass_ws(*c);
  char *word = *c;
  char *ce = get_word_end(*c);
  if(*ce != 0) *c = ce + 1; // sets c to next word
  else *c = ce;
  *ce = 0;
  if(*word == 0) seterr(ERR_INPUT);
  return word;
}

long int get_int(char *c){
  switch(*c){
    case '-':
    case '+':
      break;
    default:
      if (*c < '0' or *c > '9'){        // the strtol documentation doesn't seem to be working!!!
        raise_return(ERR_INPUT, 0);   // it won't tell me if there has been an error (I've tried with the pointer)
      }
  }
  return strtol(c, NULL, 0);
}

double get_float(char *c){
  uint8_t subt = 0;
  if(c[0] == '0' and c[1] == 'f' and 
     ((c[2] >= '0' and c[2] <= '9') or
     (c[2] == '-' or c[2] == '+'))
    ){
    c = c - subt;
    return atof((c+2));
  }
  raise_return(ERR_VALUE, 0);
}



