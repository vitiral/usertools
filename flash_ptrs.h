#ifndef outside_h
#define outside_h

#include <Arduino.h>
#include <stdlib.h>

void set_out_strs(PGM_P *s);

typedef const __FlashStringHelper *FLASH;

void *get_pointer(PGM_P s, uint8_t index, uint8_t size);
#endif
