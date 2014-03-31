#include "pt.h"

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
  clear_data();
}

PT_data *pthread::get_end(){
  //if(data == NULL) return NULL;
  PT_data *next = data;
  while(true){
    if(next->b.next == NULL) return next;
    next = next->b.next;
  }
}

void *pthread::put_data(void *putdata, uint8_t type){
  return put_data(putdata, type, 0);
}
  
void *pthread::put_data(void *putdata, uint8_t type, uint16_t len){
  PT_data *put = NULL;
  void *pdata = NULL;
  
  // Allocate correct amount of space
  switch(VTYPE(type)){
    //case(vt_int8):
    case(vt_uint8):
      //1debug("ti8");
      put = (PT_data *)malloc(sizeof(PT_data_int8));
      break;
    case(vt_int16):
    case(vt_uint16):
      //1debug("ti16");
      put = (PT_data *)malloc(sizeof(PT_data_int16));
      break;

/* Cant seem to get this one to work
    case(vt_int32):
      //1debug("ti32");
      put = (PT_data *)malloc(sizeof(PT_data_int32));
      break;
*/
    case(vt_str):
      //1debug("tstr");
      if(len == 0) len = strlen((char *) putdata) + 1;
      assert_raise(((char *)putdata)[len - 1] == 0, ERR_VALUE); //non valid string
      put = (PT_data *)malloc(sizeof(PT_data_str) + len);
      break;
      
    case(vt_pt):
      put = (PT_data *) malloc(sizeof(PT_data_pt));
      break;
    
    default:
      assert(0);
  }
  
  memcheck(put);
  
  switch(VTYPE(type)){
    //case(vt_int8):
    case(vt_uint8):
      ((PT_data_int8 *)put)->data = *((int8_t *)putdata);
      break;
    case(vt_int16):
    case(vt_uint16):
      ((PT_data_int16 *)put)->data = *((int16_t *)putdata);
      break;

/*
    case(vt_int32):
      ((PT_data_int32 *)put)->data = *((int32_t *)putdata);
*/
    case(vt_str):
      memcpy(((PT_data_str *)put)->data, putdata, len);
      break;
    
    case(vt_pt):
      ((PT_data_pt *)put)->data = *((pthread *)putdata);
      break;
      
    default:
      assert(0);
  }
  
  init_PT_data(put);
  put->b.type = type;
  if(data == NULL){
    data = (PT_data *)put;
  }
  else if(PTYPE(type) == TYPE_TEMP){
    // temp always goes in front, can only have one temp data
    assert_raise(PTYPE(data->b.type) != TYPE_TEMP, ERR_INDEX);
    put->b.next = data;
    data = (PT_data *)put;
  }
  else{
    get_end()->b.next = (PT_data *)put;
  }
  return put;
  
error:
  memclr(put);
  memclr(pdata);
  return NULL;
}

void pthread::destroy_data(PT_data *pd, PT_data *prev){
  if(pd == NULL) return; //assert(pd);
  if(prev == NULL)  data = pd->b.next;
  else              prev->b.next = pd->b.next;
  
  switch(VTYPE(pd->b.type)){
    case vt_pt:
      ((PT_data_pt *)pd)->data.clear_data();
      break;
  }
  
  free(pd);
  return;
}

int32_t pthread::get_int(PT_data_int32 *pint){
  switch(VTYPE(pint->b.type)){
    case(vt_uint8):
      return (uint8_t) pint->data;
      
    case(vt_int16):
      return (int16_t)((uint16_t)pint->data);
    case(vt_uint16):
      return (uint16_t)pint->data;
      
    //case(vt_int32):
    //  return (int32_t)(pint->data);

    default:
      raise(ERR_TYPE, pint->b.type, HEX);
  }
error:
 return 0;
}

int32_t pthread::get_int_type(ptindex index, uint8_t type){
  int32_t out;
  PT_data_int32 *mydata = (PT_data_int32 *)get_type(index, type);
  iferr_log_catch();
  out = get_int(mydata);
  iferr_log_catch();
  return out;
error:
  return 0;
}

char *pthread::get_str_type(ptindex index, uint8_t type){
  PT_data_str *mydata = (PT_data_str *)get_type(index, type);
  iferr_catch();
  assert_raise(VTYPE(mydata->b.type) == vt_str, ERR_TYPE);
  return mydata->data;
error:
  return NULL;
}

PT_data *pthread::get_type(ptindex index, uint8_t ptype){
  ptindex cur_index = 0;
  PT_data *cdata;
  assert_raise(data, ERR_INDEX);
  cdata  = data;
  
  while(true){
    if(PTYPE(cdata->b.type) == ptype) {
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

uint8_t pthread::get_type_type(ptindex index, uint8_t ptype){
  debug(get_type(index, ptype)->b.type);
  return VTYPE(get_type(index, ptype)->b.type);
}

PT_data *pthread::get_temp(uint8_t type){
  assert_raise(data, ERR_INDEX);
  assert_raise(data->b.type == TYPE_TEMP bitor type, ERR_INDEX);
  return data;
error:
  return NULL;
}

void *pthread::put_temp(uint16_t temp){
  clear_temp();
  if(put_data(&temp, TYPE_TEMP bitor vt_uint16, 0) == NULL){
    //asm volatile ("  jmp 0"); // could cause undefined behavior
    raise_return(ERR_CRITICAL, NULL);
  }
}

void *pthread::put_temp_pt(){
  clear_temp();
  if(put_data(&pthread(), TYPE_TEMP bitor vt_pt, 0) == NULL){
    //asm volatile ("  jmp 0"); // could cause undefined behavior
    raise_return(ERR_CRITICAL, NULL);
  }
}

uint16_t pthread::get_int_temp(){
  PT_data_int32 *temp = (PT_data_int32 *)get_temp(vt_uint16);
  return (uint16_t)(temp->data);
}

pthread *pthread::get_pt_temp(){
  PT_data_pt *temp = (PT_data_pt *)get_temp(vt_pt);
  return &(temp->data);
}

void pthread::clear_temp(){
  if(data != NULL){
    if(PTYPE(data->b.type) == TYPE_TEMP){
      destroy_data(data, NULL);
    }
  }
}

// *****************************************************
// **** Input

void *pthread::put_input(uint8_t input){
  //1debug("Pui8");
  put_data(&input, TYPE_INPUT bitor vt_uint8, 0);
}

void *pthread::put_input(int16_t input){
  //1debug("P_i16");
  put_data(&input, TYPE_INPUT bitor vt_int16, 0);
}

void *pthread::put_input(uint16_t input){
  //1debug("P_i16");
  put_data(&input, TYPE_INPUT bitor vt_uint16, 0);
}

//void *pthread::put_input(int32_t input){
//  //1debug("P_i32");
//  put_data(&input, TYPE_INPUT bitor vt_int32, 0);
//}

void *pthread::put_input(char *input){
  return put_data(input, TYPE_INPUT bitor vt_str);
}

void *pthread::put_input(char *input, uint16_t len){
  return put_data(input, TYPE_INPUT bitor vt_str, len);
}

int32_t pthread::get_int_input(ptindex index){
  return get_int_type(index, TYPE_INPUT);
}

char *pthread::get_str_input(ptindex index){
  return get_str_type(index, TYPE_INPUT);
}

uint8_t pthread::get_type_input(ptindex index){
  return get_type_type(index, TYPE_INPUT);
}

void pthread::clear_input(){
  clear_type(TYPE_INPUT);
}


// *****************************************************
// **** Output

void *pthread::put_output(uint8_t output){
  //1debug("Pui8");
  put_data(&output, TYPE_OUTPUT bitor vt_uint8, 0);
}

void *pthread::put_output(int16_t output){
  //1debug("P_i16");
  put_data(&output, TYPE_OUTPUT bitor vt_int16, 0);
}

void *pthread::put_output(uint16_t output){
  //1debug("P_i16");
  put_data(&output, TYPE_OUTPUT bitor vt_uint16, 0);
}

//void *pthread::put_output(int32_t output){
//  //1debug("P_i32");
//  put_data(&output, TYPE_OUTPUT bitor vt_int32, 0);
//}

void *pthread::put_output(char *output){
  return put_data(output, TYPE_OUTPUT bitor vt_str);
}

void *pthread::put_output(char *output, uint16_t len){
  return put_data(output, TYPE_OUTPUT bitor vt_str, len);
}

int32_t pthread::get_int_output(ptindex index){
  // note that integers are stored least value first
  // so I just need to convert to uintX (to break off
  // excess data) and then convert that value to int
  return get_int_type(index, TYPE_OUTPUT);
}

char *pthread::get_str_output(ptindex index){
  return get_str_type(index, TYPE_OUTPUT);
}

uint8_t pthread::get_type_output(ptindex index){
  return get_type_type(index, TYPE_OUTPUT);
}

void pthread::clear_output(){
  clear_type(TYPE_OUTPUT);
}

// *****************************************************
// **** Local


// *****************************************************
// **** Destroy / Clear

void pthread::clear_data(){
  while(data){
    destroy_data(data, NULL);
  }
}

void pthread::clear_type(uint8_t type){
  PT_data *prevdata = NULL;
  PT_data *cdata;
  cdata  = data;
  
  while(cdata){
    if(PTYPE(cdata->b.type) == type) {
      destroy_data(cdata, prevdata);
      cdata = prevdata->b.next;
    }
    else{
      prevdata = cdata;
      cdata = cdata->b.next;
    }
  }
}

  

// *****************************************************
// **** ptsmall (small protothread)

ptsmall::ptsmall(){
  lc = 0;
}

//void ptsmall::clear_data(){}
void ptsmall::clear_output(){}
