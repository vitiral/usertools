/* Copyright (c) 2014, Garrett Berg <garrett@cloudformdesign.com>, cloudformdesign.com
 * This library is released under the FreeBSD License, if you need
 * a copy go to: http://www.freebsd.org/copyright/freebsd-license.html
 * 
 * SUMMARY:
 * String tools, mostly to help with F() strings
 */
 
#ifndef strtools_h
#define strtools_h

#define cmp_str_elptr(N, L, E) __cmp_str_elptr(N, L, (TH_element *)(E))

uint16_t flash_len(const __FlashStringHelper *ifsh);

void flash_to_str(const __FlashStringHelper *ifsh, char *str);

uint8_t cmp_str_flash(char *str, const __FlashStringHelper *flsh);
uint8_t cmp_flash_flash(const __FlashStringHelper *flsh1, const __FlashStringHelper *flsh2);

char *get_word_end(char *c);
char *pass_ws(char *c);
#define get_word(C) _get_word(&(C))
#define get_int(C) _get_int(&(C))
char *_get_word(char **c);
long int _get_int(char **c);


#endif

