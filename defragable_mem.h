#ifndef _defragable_mem_h
#define _defragable_mem_h

#define DM_NULL_INDEX 255

#include <Arduino.h>
#include <inttypes.h>
#include <stdlib.h>

typedef uint8_t DM_index;

class defragable_mem{
  DM_index data_index;
  uint16_t data_taken;
  uint16_t data_max;
  uint8_t data[];
public:
  defragable_mem(uint16_t size);
  DM_index malloc(uint8_t size);
  uint8_t check_defrag(uint8_t size);
  uint8_t defrag();
  void *get_ptr(DM_index index);
  void free(DM_index index);
};
#endif
