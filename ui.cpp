
#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "LinkedList.h"

#define DEBUG
#include <SoftwareSerial.h>
#include "errorhandling.h"

#include "serialui.h"
#include "threading.h"
#include "strtools.h"

#define MAX_STR_LEN 100
#define MAX_ARRAY_LEN 256

//char *LINE_FEED = "\x0D\x0A";
char UI_CMD_END_CHAR = 0x0A;
char UI_CMD_PEND_CHAR = 0x0D;  // may be right before the end.


unsigned short function_exists(ui_funptr fun){
  for(uint8_t i = 0; i < functions.index; i++){
    assert_raise_return(functions.array[i].fptr != fun, 
    ERR_VALUE, true);
  }
  return false;
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







