

/* 
 * Notes:
 *  - [(Data Indexers --->       <---- Data Pointers}]
 *  - create data indexers by mallocing a data pointer (data_taken) and 
 *      incrememing data_index
 *  - delete data by setting it's data pointer = NULL
 *  - durring defrag only pointers set to non NULL will be moved left and preserved
 * 
 * */

#include "ReMem.h"
#include "usertools.h"
#include "errorhandling.h"

#define _DM_FRONT ((char *)data + (data_index) * sizeof(IO_indexer))
#define _DM_BACK ((char *)data + data_max)
#define _DM_DATA_FRONT (_DM_BACK - data_taken)

#define _DM_INDEX ((IO_indexer *)data)

// data_available is an 16 bit value with the following representations:
//   AA   BBB   CC   D | DD EEE FFF
//   AA - num available > 8 bytes
//   BBB - num available < 8 bytes of odd configuration (3, 5, 6, 7 bytes)
//   CC - num available == 8 bytes
//   DDD - num available == 4 bytes
//   EEE - num available == 2 bytes
//   FFF - num available == 1 byte

#define DA_GT_8         0b1100000000000000
#define DA_LT_8_ODD     0b0011100000000000
#define DA_ET_8         0b0000011000000000
#define DA_ET_4         0b0000000111000000
#define DA_ET_2         0b0000000000111000
#define DA_ET_1         0b0000000000000111

// Shift values
#define DA_GT_8_SH      14
#define DA_LT_8_ODD_SH  11
#define DA_ET_9_SH      9
#define DA_ET_4_SH      6
#define DA_ET_2_SH      3
#define DA_ET_1_SH      0

#define DA_1MATRIX      0b0100101001001001    // for adding and subtracting
#define DA_TV(DA, D)    ((DA) bitand (D))     // Used for trust testing and


#define VERY_END  ((void *)((int8_t *)data_last + abs(*(int8_t *)data_last) + 1))
#define SIZE(ptr)     (*((int8_t *)ptr - 1))

ReMem::ReMem(uint16_t size){
  data_available = 0;
  uint8_t data[size];
  data_end = (void *)((char *)data + size)
  data_last = (void *)data; // points to the size character of the last data
}

uint8_t get_id(uint8_t size){
  uint16_t id;
  switch(size){
    case 1:
      return DA_ET_1;
    case 2:
      return DA_ET_2;
    case 4:
      return DA_ET_4;
    case 8:
      return DA_ET_8;
    default:
      if(size < 8){
        return DA_LT_8_ODD;
      }
      return DA_GT_8;
  }
}

void ReMem::using_size(uint8_t size){
  uint16_t size = get_id(size);
  data_available = data_available - DA_TV(size, DA_1MATRIX);
}

void ReMem::freed_size(uint8_t size){
  uint16_t size = get_id(size);
  uint16_t temp_avail = data_available;
  // add 1 to the value
  temp_avail = temp_available + DA_TV(size, DA_1MATRIX);
  if(DA_TV(data_available)){
    if(DA_TV(temp_avail)){
      data_available = temp_avail;
    }
    // else{}  // temp_avail used to exist, now it is 0 -- overflow
  }
  else{
    data_available = temp_avail;
  }
}


void *ReMem::get_used(uint8_t size){
  // go through the whole list looking for the correct size
  // data larger than 8 bits returns the first slot that works
  // returns NULL if there are no matches
  if not(DA_TV(get_id(size), size)) return NULL;
  
  int8_t *front = data;
  if(size <= 8){
    while(front <= data_last){
      if(-(*front) == size){
        using_size(size);
        return (void *)(front + 1)
      }
      front += abs(*front) + 1
    }
  }
  else{
    while(front <= data_last){
      if(-(*front) <= size){
        return (void *)(front + 1)
      }
      front += abs(*front) + 1
    }
  }
  return NULL;
}

void *ReMem::malloc(uint8_t size){
  assert_raise(size < DM_MAX_DATA, ERR_MEMORY);
  void *put_data = get_used(size);
  if(put_data == NULL){
    // allocate new memory space
    put_data = VERY_END;
    // make sure there is enough space.
    assert_raise((char *)data_end - (char *)put_data > size + 1, ERR_MEMORY);
    
    data_last = put_data;
    
    *(int8_t *)put_data = size;
    (int8_t *)put_data ++;
  }
  return put_data;
error:
  return NULL
}

void ReMem::free(void *ptr){
  int8_t size = SIZE(ptr);
  assert(size > 0);
  SIZE(ptr) = -size;
  freed_size(size);
}

void ReMem::defrag(){
  data_available = 0;
  int8_t *front = data;
  while(front <= data_last){
    if((*front) < 0){
      freed_size(abs(*front));
    }
    front += abs(*front) + 1
  }
}
