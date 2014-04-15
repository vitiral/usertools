#ifndef _ReMem_h
#define _ReMem_h

#include <Arduino.h>
#include <inttypes.h>
#include <stdlib.h>

#define DM_MAX_DATA 127

typedef uint8_t DM_index;

class ReMem{
  uint16_t data_available;
  void *data_last;
  void *data_end;
  uint8_t data[];
  void using_size(uint8_t size);
  void freed_size(uint8_t size);
  void *get_used(uint8_t size);
  
public:
  ReMem(uint16_t size);
  void *malloc(uint8_t size);
  void free(DM_index index);
  void defrag();
};
#endif
