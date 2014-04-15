#ifndef _ReMem_h
#define _ReMem_h

#include <Arduino.h>
#include <inttypes.h>
#include <stdlib.h>

#define DM_MAX_DATA 127

class ReMem{
  uint16_t data_available;
  int8_t *data_put;
  int8_t *data_end;
  int8_t *data;
  void using_size(int16_t size);
  void freed_size(int16_t size);
  int8_t *get_used(uint8_t size);
  
public:
  //ReMem(uint16_t size);
  ReMem(void);
  void init(uint16_t size);
  void *rmalloc(uint8_t size);
  void free(void *ptr);
  void defrag();
  void print();
  void print_data();
};
#endif
