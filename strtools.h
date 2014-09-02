/* Copyright (c) 2014, Garrett Berg <garrett@cloudformdesign.com>, cloudformdesign.com
 * This library is released under the FreeBSD License, if you need
 * a copy go to: http://www.freebsd.org/copyright/freebsd-license.html
 *
 * SUMMARY:
 * String tools, mostly to help with F() strings
 */

#ifndef strtools_h
#define strtools_h
#include <usertools.h>

uint16_t flash_len(const __FlashStringHelper *ifsh);

void flash_to_str(const __FlashStringHelper *ifsh, char *str);

uint8_t cmp_str_flash(char *str, const __FlashStringHelper *flsh);
uint8_t cmp_flash_flash(const __FlashStringHelper *flsh1, const __FlashStringHelper *flsh2);

char *get_word_end(char *c);
char *pass_ws(char *c);
char *strip(char *c, uint16_t end);

#define get_word(C) _get_word(&(C))
char *_get_word(char **c);

//#define get_int(C) _get_int(&(C))
long int get_int(char *c);
//float get_float(char *c);

long int _get_int(char **c);


uint16_t bstrncpy(char **destination, const char *source, uint16_t len);
char *d2strf (double val, signed char width, unsigned char prec, char *sout);

void print_strnames();
void showString (PGM_P s);
void set_strname(PGM_P *s);

#endif

