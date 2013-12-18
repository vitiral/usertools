

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

pt::pt(){
  lc = 0;
  data = NULL;
}

pt::~pt(){
  // destroy data
}

PT_data *pt::get_end(){
  //if(data == NULL) return NULL;
  PT_data *next = data;
  while(true){
    if(next->b.next == NULL) return next;
    next = next->b.next;
  }
}

PT_data *pt::get_input(ptindex index){
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

void pt::put_data(void *ptd){
  init_PT_data(ptd);
  if(data == NULL){
    data = (PT_data *)ptd;
  }
  else{
    get_end()->b.next = (PT_data *)ptd;
  }
}

void pt::destroy_data(PT_data *pd, PT_data *prev){
  // remove from link chain
  if(prev == NULL){
    data = pd->b.next;
  }
  else{
    prev->b.next = pd->b.next;
  }
  
  // free memory
  switch(VTYPE(pint->b.type)){
    // ** Integers
    case(vt_int8):
    case(vt_uint8):
      free((PT_data_uint8) pd);
      return;
    case(vt_int16):
    case(vt_uint16):
      free((PT_data_uint16) pd);
      return
    case(vt_int32):
    case(vt_uint32):
      free((PT_data_uint32) pd);
      return
    
    case(vt_str):
      free(((PT_data_str)pd)->data);
      free((PT_data_str) pd);
      return;
    default:
      assert(0)
  }
error:
  return;
}

void pt::put_data_input(void *ptd){
  ((PT_data *)ptd)->b.type |= TYPE_INPUT;
  put_data(ptd);
}

void pt::put_data_temp(void *ptd){
  // temp data always goes in front.
  // Can only have one temp data at a time.
  init_PT_data(ptd);
  ((PT_data *)ptd)->b.type |= TYPE_TEMP;
  if(data == NULL){
    data = (PT_data *)ptd;
  }
  else{
    PT_data *first = data;
    assert_raise_return(PTYPE(first->b.type) != TYPE_TEMP, ERR_THREAD);
    // insert into front
    ((PT_data *)ptd)->b.next = first;
    data = (PT_data *)ptd;
  }
}

PT_data *pt::get_temp(){
  PT_data *td;
  assert_raise(data, ERR_THREAD);
  td = data;
  assert_raise(PTYPE(data->b.type) == TYPE_TEMP, ERR_THREAD);
  return td;
error:
  return NULL;
}

int32_t pt::get_int(PT_data_int32 *pint){
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
      return (uint16_t) pint->data;
    case(vt_uint16):
      return (uint16_t) pint->data;
    case(vt_uint32):
      return (uint32_t) pint->data;
    
    default:
      raise(ERR_TYPE);
  }
}

// *****************************************************
// **** Temporary
void pt::put_temp(uint16_t input){
  PT_data_uint16 *put = (PT_data_uint16 *) malloc(sizeof(PT_data_uint16));
  memcheck_return(put);
  put->data = input;
  put->b.type = vt_uint16;
  put_data_temp(put);
}

uint16_t pt::get_uint16_temp(){
  uint16_t out;
  PT_data_uint16 *td = get_temp();
  iferr_log_catch();
  out = td->data;
  // destroy temp()
  return out;
error:
  return 0;
}

void pt::clear_temp(){
  PT_data *td = get_temp();
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

void pt::put_input(uint8_t input){
  debug("Pui8");
  PT_data_int8 *put = (PT_data_int8 *) malloc(sizeof(PT_data_int8));
  memcheck_return(put);
  put->data = input;
  put->b.type = vt_uint8;
  put_data_input(put);
}

void pt::put_input(int16_t input){
  debug("P16");
  PT_data_int16 *put = (PT_data_int16 *) malloc(sizeof(PT_data_int16));
  memcheck_return(put);
  put->data = input;
  put->b.type = vt_int16;
  put_data_input(put);
}

int32_t pt::get_int_input(ptindex index){
  // note that integers are stored least value first
  // so I just need to convert to uintX (to break off
  // excess data) and then convert that value to int
  int32_t out;
  PT_data_int32 *mydata = (PT_data_int32 *)get_input(index);
  iferr_catch();
  out = get_int(mydata);
  iferr_log_catch();
  return out;
error:
  return 0;
}

// *****************************************************
// **** Strings

void pt::put_str_input(char *input, uint16_t len){
  PT_data_str *put = NULL;
  char *pd = NULL;
  len ++;
  put = (PT_data_str *) malloc(sizeof(PT_data_str));
  memcheck(put);
  pd = (char *) malloc(len);
  memcheck(pd);
  
  memcpy(pd, input, len);
  
  put->data = pd;
  put->b.type = vt_str;
  put_data_input(put);
  return;
error:
  memclr(put);
}

void pt::put_str_input(char *input){
  return put_str_input(input, strlen(input));
}

char *pt::get_str_input(ptindex index){
  PT_data_str *mydata = (PT_data_str *)get_input(index);
  iferr_catch();
  assert_raise(VTYPE(mydata->b.type) == vt_str, ERR_TYPE);
  return mydata->data;
error:
  return NULL;
}
