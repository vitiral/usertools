

#include "pt.h"



// commented = not supported, but may in future
enum vtype{
  //vt_int8,
  vt_uint8,
  
  vt_int16,
  vt_uint16,
  
  //vt_int32,
  //vt_uint32,
  
  vt_uint8ray,  // 8 bit uint arrays
  vt_int16ray,  // 16 bit int arrays
  
  vt_str,       // storing character arrays
  
  vt_pt,        // storing protothreads
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

void pthread::put_data(void *putdata, uint8_t type){
  return put_data(putdata, type, 0);
}
  
void pthread::put_data(void *putdata, uint8_t type, uint16_t len){
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
  return;
  
error:
  memclr(put);
  memclr(pdata);
  return;
}

void pthread::destroy_data(PT_data *pd, PT_data *prev){
  // remove from link chain
  assert(pd);
  if(prev == NULL){
    data = pd->b.next;
  }
  else{
    prev->b.next = pd->b.next;
  }
  switch(VTYPE(pd->b.type)){
    case vt_pt:
    ((PT_data_pt *)pd)->data.clear_data();
    break;
  }
  
  free(pd);
  return;
error:
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
    //  return (int32_t)((uint32_t)pint->data);

    default:
      raise(ERR_TYPE, pint->b.type, HEX);
  }
error:
 return 0;
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


// *****************************************************
// **** Temporary
// * Eventually all temporary functions will be separated
// * so that if only they are used less Flash can be 
// * used.

PT_data *pthread::get_temp(){
  assert_raise(data, ERR_INDEX);
  assert_raise(PTYPE(data->b.type) == TYPE_TEMP, ERR_INDEX);
  return data;
error:
  return NULL;
}

void pthread::put_temp(uint16_t temp){
  put_data(&temp, TYPE_TEMP bitor vt_uint16, 0);
  /*
  PT_data *put;
  put = (PT_data *)malloc(sizeof(PT_data_int16));
  memcheck(put);
  ((PT_data_int16 *)put)->data = (int16_t)temp;
  init_PT_data(put);
  put->b.type = TYPE_TEMP bitor vt_uint16;
  if(data){
    put->b.next = data;
  }
  data = put;
  return;
error:
  return;
  */
  
}

void pthread::put_temp_pt(){
  put_data(&pthread(), TYPE_TEMP bitor vt_pt, 0);
  /*
  PT_data *put;
  //pthread pt = ;
  put = (PT_data *)malloc(sizeof(PT_data_pt));
  memcheck(put);
  ((PT_data_pt *)put)->data = pthread();
  init_PT_data(put);
  put->b.type = TYPE_TEMP bitor vt_pt;
  if(data){
    put->b.next = data;
  }
  data = put;
  return;
error:
  return;
  */
}

// used for getting time
uint16_t pthread::get_int_temp(){
  PT_data_int32 *temp = (PT_data_int32 *)get_temp();
  iferr_log_catch();
  assert_raise(VTYPE(temp->b.type) == vt_uint16, ERR_TYPE);
  return (uint16_t)(temp->data);
  
  //PT_data_int32 *temp = (PT_data_int32 *)get_temp();
  //return get_int(temp);
error:
  return 0;
}

pthread *pthread::get_pt_temp(){
  PT_data_pt *temp = (PT_data_pt *)get_temp();
  iferr_log_catch();
  assert_raise(VTYPE(temp->b.type) == vt_pt, ERR_TYPE);
  return &(temp->data);
error:
  return NULL;
}

void pthread::clear_temp(){
  PT_data *temp = get_temp();
  iferr_log_return();
  destroy_data(temp, NULL);
  return;
}

// *****************************************************
// **** Input

void pthread::put_input(uint8_t input){
  //1debug("Pui8");
  put_data(&input, TYPE_INPUT bitor vt_uint8, 0);
}

void pthread::put_input(int16_t input){
  //1debug("P_i16");
  put_data(&input, TYPE_INPUT bitor vt_int16, 0);
}

void pthread::put_input(uint16_t input){
  //1debug("P_i16");
  put_data(&input, TYPE_INPUT bitor vt_uint16, 0);
}

//void pthread::put_input(int32_t input){
//  //1debug("P_i32");
//  put_data(&input, TYPE_INPUT bitor vt_int32, 0);
//}

void pthread::put_input(char *input){
  return put_data(input, TYPE_INPUT bitor vt_str);
}

void pthread::put_input(char *input, uint16_t len){
  return put_data(input, TYPE_INPUT bitor vt_str, len);
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

char *pthread::get_str_input(ptindex index){
  PT_data_str *mydata = (PT_data_str *)get_input(index);
  iferr_catch();
  assert_raise(VTYPE(mydata->b.type) == vt_str, ERR_TYPE);
  return mydata->data;
error:
  return NULL;
}

void pthread::clear_input(ptindex index){
  
}

// *****************************************************
// **** Destroy / Clear

void pthread::clear_data(){
  while(data){
    destroy_data(data, NULL);
  }
}


  

// *****************************************************
// **** psthread (small pthread)


