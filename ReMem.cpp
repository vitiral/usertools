

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

/*
#ifdef DEBUG
#undef DEBUG
#define MYDEBUG
#endif

#define LOGLEVEL 1
*/

#include "errorhandling.h"


// data_available is an 16 bit value with the following representations:
//   AA   BBB   CC   D | DD EEE FFF
//   AA - num available > 8 bytes
//   BBB - num available < 8 bytes of odd configuration (3, 5, 6, 7 bytes)
//   CC - num available == 8 bytes
//   DDD - num available == 4 bytes
//   EEE - num available == 2 bytes
//   FFF - num available == 1 byte

#define DA_DEFRAG       0b1000000000000000    // indicates that data needs to be defraged
#define DA_GT_8         0b0110000000000000
#define DA_LT_8_ODD     0b0001110000000000
#define DA_ET_8         0b0000001100000000
#define DA_ET_4         0b0000000011000000
#define DA_ET_2         0b0000000000111000
#define DA_ET_1         0b0000000000000111
#define DA_1MATRIX      0b0010010101001001    // for adding and subtracting
//#define DA_TV(DA, D)    ((uint16_t)((DA) bitand (D)))    // Used for trust testing and
#define DA_TV(DA, D)    (((uint16_t)(DA) bitand (uint16_t)(D)))    // Used for trust testing and


#define SIZE(ptr)     (*((int8_t *)ptr - 1))
#define DATA_REMAINING     (((uint16_t)data_end - (uint16_t)data_put))
#define TOTAL_SIZE         (((uint16_t)data_end - (uint16_t)data))
//ReMem::ReMem(uint16_t size){
ReMem::ReMem(void){
  data_available = 0;
}

void ReMem::init(uint16_t size){
  data = (int8_t *) malloc(size);
  debug((uint16_t)data);
  // do error checking
  data_end = data + size;
  data_put = data; // points to the size character of the last data
}

int16_t get_id(uint8_t size){
  switch(size){
    case 0:
      assert_return(0, 0);
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

void ReMem::using_size(int16_t size){
  size = get_id(size);
  data_available = data_available - DA_TV(size, DA_1MATRIX);
}

int8_t *ReMem::get_used(uint8_t size){
  // go through the whole list looking for the correct size
  // data larger than 8 bits goes into any slot <= 1.5 times it's size
  // returns NULL if there are no matches
  //sdebug("Gu:"); edebug(size);
  if(not DA_TV(get_id(size), data_available)) return NULL;

  int8_t *front = data;
  if(size <= 8){
    while(front < data_put){
      //debug((uint16_t) front);
      if(-(*front) == size){
        using_size(size);
        *front = -(*front);
        return front + 1;
      }
      //debug(*front);
      front += abs(*front) + 1;
    }
  }
  else{
    //debug((uint16_t) front);
    while(front < data_put){
      if(size <= -(*front)){ // must be able to fit
        if(size + size/2 >= -(*front)){  // shouldn't take up a slot MUCH bigger than it
          using_size(size);
          *front = -(*front);
          return front + 1;
        }
      }
      front += abs(*front) + 1;
    }
  }

  //TODO: canibilize large sizes
  return NULL;
}

void *ReMem::rmalloc(uint8_t size){
  int8_t *put;

  assert_raise(size < DM_MAX_DATA, ERR_MEMORY);
  put = get_used(size);

  if(put == NULL){
    // allocate new memory space
    put = data_put;
    //debug((uint16_t) put);

    // make sure there is enough space.
    assert_raise(DATA_REMAINING > size + 1, ERR_MEMORY);

    data_put = put + size + 1;
    //debug(size);
    //debug((uint16_t) put);
    //debug((uint16_t) data_put);

    //debug((uint16_t)(put + 1));

    //debug(size);
    put[0] = size;
    //debug(put[0]);
    put++;
  }
  sdebug(F("MAL:")); cdebug(size); cdebug(F("\tp:")); edebug((uint16_t) put);
  //debug(SIZE(put));
  return (void *)put;
error:
  clrerr();
  return NULL;
}

void ReMem::freed_size(int16_t size){
  size = get_id(size);
  // add 1 to the value
  uint16_t temp_avail = data_available + (uint16_t)DA_TV(size, DA_1MATRIX);
  debug(temp_avail, BIN);
  if(DA_TV(size, data_available)){
    if(DA_TV(size, temp_avail)){
      data_available = temp_avail;
    }
    else{ // temp_avail used to exist, now it is 0 -- overflow
      data_available |= DA_DEFRAG;
    }
  }
  else{
    data_available = temp_avail;
  }
}

uint8_t ReMem::free(void *ptr){
  // returns 1 on success, 0 on failure
  sdebug(F("F:")); cdebug('\t'); cdebug((uint16_t) ptr); cdebug('\t');
  edebug(SIZE(ptr));
  if(not ((data < ptr) and (ptr < data_put))) return 0;
  assert_raise_return((data < ptr) and (ptr < data_put), ERR_MEMORY, 0);

  int8_t size = SIZE(ptr);
  assert_return(size > 0, 0);

  SIZE(ptr) = -size;
  freed_size(size);
  return 1;
}

void ReMem::defrag(){
  if(not DA_TV(data_available, DA_DEFRAG)) return;

  data_available = 0;
  int8_t *front = data;
  while(front < data_put){
    if((*front) < 0){
      freed_size(abs(*front));
    }
    front += abs(*front) + 1;
  }
}

void ReMem::print(){
  L_print(F("ReMem:"));
  L_print(DATA_REMAINING);
  L_print(F("\t0b")); L_print(data_available, BIN);
  L_print(F("\tD:")); L_print((uint16_t) data);
  L_print(F("\tP:")); L_print((uint16_t) data_put);
  L_print(F("\tE:")); L_println((uint16_t) data_end);

  L_println();
}

void ReMem::print_data(){
  uint8_t i;
  int8_t *front = data;
  while(front < data_put) {
    L_print(*front); L_print(F("\tx"));
    for(i = 1; i < abs(*front) + 1; i++){
      L_write(' '); L_print(front[i], HEX);
    }
    if(abs(*front) > 8){
      L_println(); L_write('\t');
      L_print((char *)(front + 1));
    }
    front += abs(*front) + 1;
    L_println();
  }
}
