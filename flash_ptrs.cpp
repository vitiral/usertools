
#include "flash_ptrs.h"

void *get_pointer(PGM_P s, uint8_t index, uint8_t size){
  return (void *)pgm_read_word(s + index * size);
}

PGM_P *out_strs = NULL;
void set_out_strs(PGM_P *s) {
  out_strs = s;
  Serial.println("GOT");
  //Serial.println((const __FlashStringHelper **)out_strs[0]);
    Serial.println((const __FlashStringHelper *)get_pointer((PGM_P)out_strs, 0, sizeof(PGM_P)));
    Serial.println((const __FlashStringHelper *)get_pointer((PGM_P)out_strs, 1, sizeof(PGM_P)));
    Serial.println((const __FlashStringHelper *)get_pointer((PGM_P)s, 0, sizeof(PGM_P)));
    Serial.println((const __FlashStringHelper *)get_pointer((PGM_P)s, 1, sizeof(PGM_P)));
}
