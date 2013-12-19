

#include "pt.h"

enum vtype{
  vt_uint8,
  vt_uint16,
  vt_uint32,
  vt_int8,
  vt_int16,
  vt_int32,
  
  vt_str,
  
};

#define TYPE_BITSHIFT 6
#define TYPE_ERROR    0 << TYPE_BITSHIFT
#define TYPE_INPUT    1 << TYPE_BITSHIFT 
#define TYPE_OUTPUT   2 << TYPE_BITSHIFT 
#define TYPE_TEMP     3 << TYPE_BITSHIFT 

#define PTYPE_MASK 0b11 << TYPE_BITSHIFT 
#define PTYPE(D) (PTYPE_MASK bitand (D))  

#define VTYPE_MASK 0b111111
#define VTYPE(D) (VTYPE_MASK bitand (D))



// *****************************************************
// **** Private Class Helpers

void init_PT_data(void *pd){
  ((PT_data *)pd)->b.next = NULL;
}

pthread::pthread(){
  lc = 0;
  data = NULL;
}

pthread::~pthread(){
  // destroy data
}

PT_data *pthread::get_end(){
  //if(data == NULL) return NULL;
  PT_data *next = data;
  while(true){
    if(next->b.next == NULL) return next;
    next = next->b.next;
  }
}

PT_data *pthread::get_input(ptindex index){
  ptindex cur_index = 0;
  PT_data *cdata;
  assert_raise(data, ERR_INDEX);
  cdata  = data;
  
  while(true){
    if(PTYPE(cdata->b.type) == TYPE_INPUT) {
      if(index == cur_index){
        return cdata;
      }
      cur_index++;
    }
    cdata = cdata->b.next;
    assert_raise(cdata, ERR_INDEX);
  }
error:
  return NULL;
}

void pthread::put_data(void *putdata, uint8_t type){
  return put_data(putdata, type, 0);
}
  
void pthread::put_data(void *putdata, uint8_t type, uint16_t len){
  PT_data *put = NULL;
  void *pdata = NULL;
  
  // Allocate correct amount of space
  switch(VTYPE(type)){
    case(vt_int8):
    case(vt_uint8):
      debug("ti8");
      put = (PT_data *)malloc(sizeof(PT_data_int8));
      break;
    case(vt_int16):
    case(vt_uint16):
      debug("ti16");
      put = (PT_data *)malloc(sizeof(PT_data_int16));
      break;
      
    case(vt_str):
      debug("tstr");
      put = (PT_data *)malloc(sizeof(PT_data_str));
      break;
    default:
      assert(0);
  }
  
  memcheck(put);
  
  switch(VTYPE(type)){
    case(vt_int8):
    case(vt_uint8):
      ((PT_data_int8 *)put)->data = *((int8_t *)putdata);
      break;
    case(vt_int16):
    case(vt_uint16):
      ((PT_data_int16 *)put)->data = *((int16_t *)putdata);
      break;
      
    case(vt_str):
      if(len == 0) len = strlen((char *) putdata) + 1;
      assert_raise(((char *)putdata)[len - 1] == 0, ERR_VALUE); //non valid string
      pdata = malloc(len);
      memcheck(pdata);
      memcpy(pdata, putdata, len);
      ((PT_data_str *)put)->data = (char *)pdata;
      break;
    default:
      assert(0);
  }
  
  init_PT_data(put);
  put->b.type = type;
  if(data == NULL){
    data = (PT_data *)put;
  }
  else{
    get_end()->b.next = (PT_data *)put;
  }
  return;
  
error:
  memclr(put);
  memclr(pdata);
  return;
}

void pthread::destroy_data(PT_data *pd, PT_data *prev){
  // remove from link chain
  if(prev == NULL){
    data = pd->b.next;
  }
  else{
    prev->b.next = pd->b.next;
  }
  
  // free memory
  switch(VTYPE(pd->b.type)){
    // ** Integers
    case(vt_int8):
    case(vt_uint8):
      free((PT_data_int8 *) pd);
      return;
    case(vt_int16):
    case(vt_uint16):
      free((PT_data_int16 *) pd);
      return;
    case(vt_int32):
    case(vt_uint32):
      free((PT_data_int32 *) pd);
      return;
      
    case(vt_str):
      free(((PT_data_str *)pd)->data);
      free((PT_data_str *) pd);
      return;
    default:
      assert(0, pd->b.type);
  }
error:
  return;
}

PT_data *pthread::get_temp_object(){
  PT_data *td;
  assert_raise(data, ERR_THREAD);
  td = data;
  assert_raise(PTYPE(data->b.type) == TYPE_TEMP, ERR_THREAD);
  return td;
error:
  return NULL;
}

int32_t pthread::get_int(PT_data_int32 *pint){
  switch(VTYPE(pint->b.type)){
    // signed
    case(vt_int8):
      return (int8_t) ((uint8_t)pint->data);
    case(vt_int16):
      return (int16_t) ((uint16_t)pint->data);
    case(vt_int32):
      return (int32_t) ((uint32_t)pint->data);
    
    // unsigned
    case(vt_uint8):
      return (uint8_t) pint->data;
    case(vt_uint16):
      return (uint16_t) pint->data;
    case(vt_uint32):
      return (uint32_t) pint->data;
    
    case(vt_str):
      assert(0);
    
    default:
      raise(ERR_TYPE, pint->b.type, HEX);
  }
error:
 return 0;
}

// *****************************************************
// **** Temporary
void pthread::put_temp(uint16_t input){
  put_data(&input, TYPE_TEMP bitor vt_uint16);
}

uint16_t pthread::get_temp(){
  int16_t out;
  PT_data_int16 *td = (PT_data_int16 *) get_temp();
  iferr_log_catch();
  out = (uint16_t)(td->data);
  // destroy temp()
  return out;
error:
  return 0;
}

void pthread::clear_temp(){
  PT_data *td = get_temp_object();
  iferr_log_catch();
  destroy_data(td, NULL);
error:
  return;
}

// *****************************************************
// **** Integers
// * Put integers with put_intXX_input()
// * Get integers with get_int_input()    // no need for xx

// puts an integer into the data list, allocating
// space for it.


void pthread::put_input(uint8_t input){
  debug("Pui8");
  put_data(&input, TYPE_INPUT bitor vt_uint8, 0);
}

void pthread::put_input(int16_t input){
  debug("P16");
  put_data(&input, TYPE_INPUT bitor vt_int16, 0);
}

int32_t pthread::get_int_input(ptindex index){
  // note that integers are stored least value first
  // so I just need to convert to uintX (to break off
  // excess data) and then convert that value to int
  int32_t out;
  PT_data_int32 *mydata = (PT_data_int32 *)get_input(index);
  iferr_log_catch();
  out = get_int(mydata);
  iferr_log_catch();
  return out;
error:
  return 0;
}

// *****************************************************
// **** Strings

void pthread::put_str_input(char *input){
  return put_data(input, TYPE_INPUT bitor vt_str);
}

void pthread::put_str_input(char *input, uint16_t len){
  return put_data(input, TYPE_INPUT bitor vt_str, len);
}

char *pthread::get_str_input(ptindex index){
  PT_data_str *mydata = (PT_data_str *)get_input(index);
  iferr_catch();
  assert_raise(VTYPE(mydata->b.type) == vt_str, ERR_TYPE);
  return mydata->data;
error:
  return NULL;
}




// *****************************************************
// **** psthread (small pthread)


