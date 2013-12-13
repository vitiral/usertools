
#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "LinkedList.h"
#include <pt.h>

#define DEBUG
#include <SoftwareSerial.h>
#include "errorhandling.h"

#include "serialui.h"

#define MAX_STR_LEN 100
#define MAX_ARRAY_LEN 256

//char *LINE_FEED = "\x0D\x0A";
char UI_CMD_END_CHAR = 0x0A;
char UI_CMD_PEND_CHAR = 0x0D;  // may be right before the end.

// #####################################################
// ### Struct Declaration

typedef uint8_t (*ui_funptr)(struct pt *pt, char *input);

class ui_thread
{
public:
  ui_thread(ui_function *func);
  ui_thread();
  ui_function             *function;
};

ui_thread::ui_thread(ui_function *func){
  function = func;
}

ui_thread::ui_thread(){
  function = NULL;
}

typedef struct Variables{
  ui_variable *array;
  uint8_t len;
  uint16_t index;
};

typedef struct Functions{
  struct ui_function *array;
  uint8_t len;
  uint16_t index;
};


// #####################################################
// ### Globals
Variables variables = {
  0, 0, 0};
Functions functions = {
  0, 0, 0};
LinkedList<ui_thread> threads = LinkedList<ui_thread>();


// #####################################################
// ### Useful Functions
uint16_t fstr_len(const __FlashStringHelper *ifsh){
  const char PROGMEM *p = (const char PROGMEM *)ifsh;
  size_t n = 0;
  while (1) {
    unsigned char c = pgm_read_byte(p++);
    if (c == 0) break;
    n++;
  }
  return n;
}

uint8_t cmp_str_flash(char *str, const __FlashStringHelper *flsh){
  const char PROGMEM *p = (const char PROGMEM *)flsh;
  char *c;
  size_t n = 0;
  while (c != 0) {
    unsigned char c = pgm_read_byte(p++);
    if (c != str[n]) return false;
    n++;
  }
  return true;
}

uint8_t cmp_flash_flash(const __FlashStringHelper *flsh1, const __FlashStringHelper *flsh2){
  char *c1, *c2;
  const char PROGMEM *p1 = (const char PROGMEM *)flsh1;
  const char PROGMEM *p2 = (const char PROGMEM *)flsh2;
  while (c1 != 0) {
    unsigned char c1 = pgm_read_byte(p1++);
    unsigned char c2 = pgm_read_byte(p2++);
    if (c1 != c2) return false;
  }
  return true;
}

// cmp_str_el(string, len, element) -- compares the string with length len to the element
#define cmp_str_elptr(N, L, E) __cmp_str_elptr(N, L, (ui_element *)(E))
uint8_t __cmp_str_elptr(char *name, uint16_t name_len, ui_element *el){
  debug(String("cmp:") + name);
  debug(el->name);
  debug(String(name_len) + String(" ") + String(el->name_len));
  if(el->name_len != name_len) return false;
  debug("len same");
  if(cmp_str_flash(name, el->name)) return true; 
  else return false;
}

//compare flash helper with element pointer
uint8_t cmp_flhp_elptr(const __FlashStringHelper *flph, uint8_t len, ui_element *el){
  debug("cmp flhp");
  debug(flph);
  debug(el->name);
  if(el->name_len != len) return false;
  if(!cmp_flash_flash(flph, el->name)) return false;
  debug("equal");
  return true;
}

unsigned short function_exists(ui_funptr fun){
  for(uint8_t i = 0; i < functions.index; i++){
    assert_raise_return(functions.array[i].fptr != fun, 
    ERR_VALUE, true);
  }
  return false;
}

// #####################################################
// ### Macro Helpers
/*
void UI__init_threads(thr){
 //LinkedList<ui_function> *threads = new LinkedList<ui_function>();
 //LinkedList<int> *threads = new LinkedList<int>();
 threads = thr;
 }
 */

void UI__set_variable_array(ui_variable *vray, uint16_t len){
  assert_raise_return(len < MAX_ARRAY_LEN, ERR_VALUE);
  variables.array = vray;
  variables.len = len;
  variables.index = 0;
}

void UI__expose_variable(const __FlashStringHelper *name, void *varptr, uint8_t varsize){
  assert_return(variables.index <= variables.len);
  assert_return(fstr_len(name) <= MAX_STR_LEN);
  assert_raise_return(variables.array, ERR_PTR); // assert not null
  uint8_t len = fstr_len(name);
  assert_raise_return(len > 0, ERR_SIZE);

  variables.array[variables.index].el.name = name;
  variables.array[variables.index].el.name_len = len;
  variables.array[variables.index].vptr = varptr;
  variables.array[variables.index].size = varsize;
  debug("Added V");
  debug(name);
  debug(String("len:") + String(variables.array[variables.index].el.name_len));
  variables.index++;
}

void UI__set_function_array(struct ui_function *fray, uint16_t len){
  assert_raise_return(len < MAX_ARRAY_LEN, ERR_VALUE);
  functions.array = fray;
  functions.len = len;
}

ui_function *UI__expose_function(const __FlashStringHelper *name, ui_funptr fptr){
  debug("ExpFun");
  assert_return(functions.index <= functions.len, NULL);
  assert_return(fstr_len(name) <= MAX_STR_LEN, NULL);
  assert_raise_return(functions.array, ERR_PTR, NULL); // assert not null

  uint8_t len = fstr_len(name);
  assert_raise_return(len > 0, ERR_SIZE, NULL);

  functions.array[functions.index].el.name = name;
  functions.array[functions.index].el.name_len = len;
  functions.array[functions.index].fptr = fptr;

  struct pt pt;
  pt.lc = -1;
  pt.error = 0;
  pt.time = 0;
  functions.array[functions.index].pt = pt;
  debug("Added F");
  debug(name);
  debug(String("len:") + String(len));
  debug(functions.array[functions.index].pt.lc);
  functions.index++;
  return &(functions.array[functions.index - 1]);
}

#define UI_STD_VARLEN 20
#define UI_STD_FUNLEN 20
void ui_setup_std(){
  static unsigned short run = false;
  assert_return(run == false);
  run = true;
  static ui_variable UI__variable_array[UI_STD_VARLEN]; 
  static uint8_t UI__variable_len = UI_STD_VARLEN;
  ui_setup_variables();

  static ui_function UI__function_array[UI_STD_FUNLEN];
  static uint8_t UI__function_len = UI_STD_FUNLEN;
  ui_setup_functions();
}


// #####################################################
// ### Functions
uint8_t print_variable(char *name){
  ui_variable *var;
  int8_t n;
  uint8_t i;
  uint8_t name_len = strlen(name);
  debug(String("pv name: ") + String(name));
  for(i = 0; i < variables.index; i++){
    var = &variables.array[i];
    debug(var->el.name);
    //if(__cmp_str_elptr_v(name, name_len, var)){
    if(cmp_str_elptr(name, name_len, var)){
      debug("matched");
      Serial.print(F("v=x"));
      for(n = var->size - 1; n >= 0; n--){
        uint8_t *chptr = (uint8_t*)(var->vptr);  // works
        if(chptr[n] < 0x10) Serial.write('0');
        Serial.print(chptr[n], HEX);
      }
      Serial.println();
      return true;
    }
  }
  return false; 
}


char *UI_TABLE_SEP = " \t ";

void _print_threads(){
  debug("printing Threads");
  Serial.println(F("Running Threads:"));
  Serial.println(F("Name\t\tLine\t\tError"));
  uint8_t i = 0;
  ui_function *f;
  while(i < threads.size()){
    f = threads.get(i).function;
    Serial.print(f->el.name);
    Serial.print(UI_TABLE_SEP);
    Serial.print((unsigned int)f->pt.lc);
    Serial.print(UI_TABLE_SEP);
    Serial.println(f->pt.error);
  }
}

uint8_t print_threads(struct pt *pt, char *input){
  static uint8_t print_periodically = false;
  if(input) _print_threads();

  char *word = get_word(input);
  iferr_log_catch();
  if(cmp_str_flash(word, F("on"))) print_periodically = true;
  if(cmp_str_flash(word, F("off"))) print_periodically = false;

error:
  PT_BEGIN(pt);
  while(true){
    if(print_periodically) _print_threads();
    PT_WAIT_MS(pt, 5000);
  }
  PT_END(pt);
}

// schedule an initialized function
// returns 0 on error
uint8_t schedule_function(ui_function *fun, char *input) {
  uint8_t out;
  debug("Calling");
  debug(fun->pt.lc);
  assert_raise_return(fun->pt.lc == -1, ERR_THREAD, 0);
  PT_INIT(&(fun->pt));
  out = fun->fptr(&(fun->pt), input);
  if(out >= PT_EXITED){
    return 1;
  }
  else{
    threads.add(ui_thread(fun));
    return 1;
  }
}

// scehdule an uninitilized function
ui_function *schedule_function(const __FlashStringHelper *name, ui_funptr fun){
  debug("Scheduling raw");
  debug(name);
  clrerr();
  ui_function *uifun = UI__expose_function(name, fun);
  iferr_log_return(NULL);
  debug("Check name:");
  debug(uifun->el.name);
  debug(uifun->pt.lc);
  schedule_function(uifun, EH_EMPTY_STR);
  return uifun;
}

uint8_t call_function(char *name, char *input){
  struct ui_function *var;
  int8_t n;
  uint8_t i;
  uint8_t name_len = strlen(name);

  debug(String("cf name: ") + String(name));
  for(i = 0; i < variables.index; i++){
    var = &functions.array[i];
    debug(var->el.name);
    if(cmp_str_elptr(name, name_len, var)){
      debug("matched");
      return schedule_function(var, input);
    }
  }
  return 0; 
}

void function_innactive(ui_function *f){
  f->pt.lc = -1;
}

struct ui_buffer{
  char buffer[MAX_STR_LEN + 2];
  uint8_t i;
} 
ui_buffer;

char *pass_ws(char *c){
  while(*c == ' ') {
    c++;
  }
  return c;
}

char *get_cmd_end(char *c){
  uint8_t k = false;
  while(*c != UI_CMD_END_CHAR){
    c++;
    k = true;
  }
  if(k){ // if the prev char is 0x0A, make that the end.
    if(*(c-1) == UI_CMD_PEND_CHAR) c -= 1;
  }
  assert_return(c, 0);
  return c;
}

char *get_word_end(char *c){
  c = pass_ws(c);
  while(*c != ' ' and *c != 0 and *c != 0x0A and *c != 0x0D){
    c++;
  }
  return c;
}

//char *word = get_word(c); // get first word
//char *word2 = get_word(c); // get next word
char *_get_word(char **c){
  *c = pass_ws(*c);
  char *word = *c;
  char *ce = get_word_end(*c);
  debug(ce - 1);
  *ce = 0;
  *c = ce + 1; // sets c to next word
  if(*word == 0) seterr(ERR_INPUT);
  return word;
}

long int _get_int(char **c){
  char *word = _get_word(c);
  iferr_log_return(0);
  return strtol(*c, NULL, 0);
}

void ui_process_command(){
  char *c = ui_buffer.buffer;
  char *c2;
  char *word;
  debug("Parse CMD");
  debug(*c);
  c = pass_ws(c);
  debug(*c);

  if(*c == '?'){
    c++;
    switch(*c){
    case 'p':
      //return value at pin #
      break;
    case 'v':
      debug("cmd 'v'");
      word = get_word(c); // skip first word, it is just 'v'
      iferr_log_catch();
      word = get_word(c);
      debug(String("var:") + word);
      print_variable(word);
      break;
    case 'f':
      // print out all exposed functions
      break;
    case 't':
      // print out running threads
      break;
    }   
  }

error:
  return;
}

void clr_ui_buff(){
  ui_buffer.i = 0;
  ui_buffer.buffer[0] = 0;
}

void ui_interface(){
  uint8_t v;
  char c;
  Serial.setTimeout(0);
  if(Serial.available()){
    debug("got in");
    while(Serial.available()){
      c = Serial.read();
      ui_buffer.buffer[ui_buffer.i] = c;
      if(ui_buffer.i > MAX_STR_LEN){
        debug("Max Len, clearing buffer");
        while(Serial.available()) Serial.read();
        clr_ui_buff();
        raise(ERR_COMMUNICATION, String(MAX_STR_LEN) + "ch MAX");
      }
      else if(ui_buffer.buffer[ui_buffer.i] == UI_CMD_END_CHAR){
        ui_buffer.buffer[ui_buffer.i + 1] = 0;
        debug(String("Command:") + ui_buffer.buffer);
        ui_process_command();
        if(Serial.available())
          debug(Serial.peek(), HEX);
        clr_ui_buff();
        goto done;
      } 
      ui_buffer.i += 1;
      ui_buffer.buffer[ui_buffer.i] = 0;
    }

    debug(String("buff size:") + String(ui_buffer.i));
    debug(ui_buffer.buffer[ui_buffer.i-2], HEX);
    debug(ui_buffer.buffer[ui_buffer.i-1], HEX);
    debug(ui_buffer.buffer[ui_buffer.i], HEX);

  }
done:
error:
  // may want to do final stuff in the future (reset timeout)
  return;
}

void UI__setup_ui(){
  ui_buffer.i = 0;
  ui_buffer.buffer[0] = 0;
}

uint8_t ui_loop(){
  static int16_t i = 0;
  ui_function *f;
  PT_LOCAL_BEGIN(pt);
  while(true){
    ui_interface();
    PT_YIELD(pt);
    i = 0;
    while(i < threads.size()){
      f = threads.get(i).function;
      if(f->fptr(&(f->pt), EH_EMPTY_STR) >= PT_EXITED){
        function_innactive(f);
        threads.remove(i);
        i -= 1;
      }
      clrerr();
      i += 1;
      PT_YIELD(pt);
    }
  }
  PT_END(pt);
}







