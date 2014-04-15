

/* 
 * Notes:
 *  - [(Data Indexers --->       <---- Data Pointers}]
 *  - create data indexers by mallocing a data pointer (data_taken) and 
 *      incrememing data_index
 *  - delete data by setting it's data pointer = NULL
 *  - durring defrag only pointers set to non NULL will be moved left and preserved
 * 
 * */

#include "defragable_mem.h"
#include "usertools.h"
#include "errorhandling.h"

#define _DM_FRONT ((char *)data + (data_index) * sizeof(IO_indexer))
#define _DM_BACK ((char *)data + data_max)
#define _DM_DATA_FRONT (_DM_BACK - data_taken)

#define _DM_INDEX ((IO_indexer *)data)

typedef struct IO_indexer{
  uint8_t size;
  void *data;
};

#define GET_SIZE(index)  (((IO_data_variable *)get_data(index))->size)

defragable_mem::defragable_mem(uint16_t size){
  data_index = 0;
  data_taken = 0;
  data_max = size;
  uint8_t data[size];
}

DM_index defragable_mem::malloc(uint8_t size){
  assert_raise_return(check_defrag(size), ERR_MEMORY, DM_NULL_INDEX);
  data_taken += size;
  _DM_INDEX[data_index].size = size;
  _DM_INDEX[data_index].data = (void *)_DM_DATA_FRONT;
  data_index += 1;
  return data_index - 1;
}

uint8_t defragable_mem::check_defrag(uint8_t size){
  // 1 = fine or can be defragged
  if(data_taken + data_index*sizeof(IO_indexer) + size + 1  > data_max){
    return 1;
  }
  return defrag();
}

uint8_t defragable_mem::defrag(){
  // performs defrag returns 1. If defrag can't be done returns 0
  // move all data to the left, ignoring data to be deleted.
  // move all data back to the right.
  // reconstruct data pointers.
  char *front = _DM_FRONT;
  char *back = _DM_DATA_FRONT;
  char *indexer_front = (char *)data;
  
  DM_index index;
  assert_return(index, 0);
  
  // move all non-deleted data to the left side
  for(index = 0; index < data_index; index++){
    if(_DM_INDEX[index].data != NULL){
      memcpy((void *)front, (void *) back, _DM_INDEX[index].size);
      memcpy((void *)indexer_front, &(_DM_INDEX[index]), sizeof(IO_indexer));
      front += _DM_INDEX[index].size;
      back += _DM_INDEX[index].size;
      indexer_front += sizeof(IO_indexer);
    }
    else{
      data_index -= 1;
      data_taken -= _DM_INDEX[index].size;
    }
  }
  
  // remember that the first data point is on the right side
  back = _DM_BACK;
  
  // move data back to the right side, set pointers correctly
  for(index = 0; index < data_index; index++){
    front -= _DM_INDEX[index].size;
    back -= _DM_INDEX[index].size;
    memcpy((void *)front, (void*)back, _DM_INDEX[index].size);
    _DM_INDEX[index].data = back;
  }
  return 1;
}

void *defragable_mem::get_ptr(DM_index index){
  assert_return(index < data_index, NULL);
  return _DM_INDEX[index].data;
}

void defragable_mem::free(DM_index index){
  _DM_INDEX[index].data = NULL;
}
