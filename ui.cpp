
#include "usertools.h"

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <SoftwareSerial.h>
#include "errorhandling.h"

#include "ui.h"
#include "threading.h"
#include "strtools.h"
#include "MemoryFree.h"



//const __FlashStringHelper *UI_TABLE_SEP = PROGMEM(" \t ");


// #####################################################
// ### Globals

#define MAX_STR_LEN 100

uint8_t print_periodically = false; // used by system_monitor

// #####################################################
// ### Functions

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
  //debug(ce);
  if(*ce != 0) *c = ce + 1; // sets c to next word
  else *c = ce;
  *ce = 0;
  if(*word == 0) seterr(ERR_INPUT);
  return word;
}

long int _get_int(char **c){
  char *word = _get_word(c);
  iferr_log_return(0);
  return strtol(*c, NULL, 0);
}

void ui_process_command(char *c){
  char *c2;
  char *word;
  sdebug(F("Parse CMD:"));
  edebug(*c);
  c = pass_ws(c);
  //debug(*c);
  c2 = get_word(c);
  iferr_log_return();
  assert_return(c); assert_return(c2);
  sdebug(F("Calling Function:"));
  cdebug(c2); cdebug(':');
  edebug(c);
  ui_call_name(c2, c);
}

void cmd_v(char *input){
  char *word = get_word(input);
  iferr_log_return();
  print_variable(word);

}

void cmd_kill(char *input){
  char *word = get_word(input);
  iferr_log_return();
  sdebug(F("killing:")); edebug(word);
  thread *th = TH_get_thread(word);
  debug(th->el.name);
  assert_raise_return(th, ERR_INPUT);
  kill_thread(th);
  Serial.print(F("Killed:"));
  Serial.println(word);

}

void _print_monitor(uint32_t execution_time){
  Serial.print(F("Total Time: "));
  Serial.print(execution_time);
  Serial.print(F("Free Memory:"));
  Serial.println(freeMemory());

  Serial.println(F("Name\t\tExTime\t\tLine"));
  uint8_t i = 0;
  thread *th;
  while(i < TH__threads.index){
    th = &TH__threads.array[i];
    if(th->pt.lc >= PT_KILL_VALUE) continue;
    Serial.print(th->el.name);
    Serial.print(UI_TABLE_SEP);
    Serial.print(th->time / 100);
    Serial.write('.');
    Serial.print(th->time % 100);
    Serial.print(UI_TABLE_SEP);
    Serial.print((unsigned int)th->pt.lc);
    //Serial.print(UI_TABLE_SEP);
    //Serial.println(th->pt.error);
    
    th->time = 0; // reset time
    i++;
  }
}

void monswitch(char *input){
  char *word = get_word(input);
  assert_raise(word, ERR_INPUT);
  iferr_log_return();
  
  if(cmp_str_flash(word, F("on"))) {
    debug(F("=on"));
    print_periodically = true;
  }
  else if(cmp_str_flash(word, F("off"))){
    debug(F("=off"));
    print_periodically = false;
  }
  else goto error;
error:
  debug(F("VI:on,off"));
}

uint8_t system_monitor(pthread *pt, char *input){
  static uint16_t time;
  PT_BEGIN(pt);
  while(true){
    PT_WAIT_MS(pt, time, 5000);
    if(print_periodically) _print_monitor((uint32_t)millis() - time);
  }
  PT_END(pt);
}

void print_options(char *input){
  uint8_t i = 0;
  Serial.println(F("Threads"));
  for(i = 0; i < TH__threads.index; i++){
    Serial.print(i);
    Serial.write('\t');
    Serial.println(TH__threads.array[i].el.name);
  }
  
  Serial.println(F("\nVars"));
  for(i = 0; i < TH__variables.index; i++){
    Serial.println(TH__variables.array[i].el.name);
  }
  
  Serial.println(F("\nFuncs"));
  for(i = 0; i < TH__functions.index; i++){
    Serial.println(TH__functions.array[i].el.name);
  }
}

uint8_t print_variable(char *name){
  TH_variable *var;
  int8_t n;
  uint8_t i;
  uint8_t name_len = strlen(name);
  var = TH_get_variable(name);
  assert_raise_return(var, ERR_INPUT, false);
  Serial.print(F("v=x"));
  for(n = var->size - 1; n >= 0; n--){
    uint8_t *chptr = (uint8_t*)(var->vptr);  // works
    if(chptr[n] < 0x10) Serial.write('0');
    Serial.print(chptr[n], HEX);
  }
  Serial.println();
  return true;
}

uint8_t user_interface(pthread *pt, char *input){
  uint8_t v;
  static uint8_t i = 0;
  char c;
  static char *buffer;
  
  if(buffer == NULL){
    buffer = (char *)malloc(MAX_STR_LEN);
    buffer[0] = 0;
  }
  
  if(Serial.available()){
    while(Serial.available()){
      c = Serial.read();
      buffer[i] = c;
      if(i > MAX_STR_LEN){
        while(Serial.available()) Serial.read();
        raise(ERR_COMMUNICATION, F("Size"));
      }
      else if(buffer[i] == UI_CMD_END_CHAR){
        buffer[i + 1] = 0;
        sdebug(F("Command:"));
        edebug(buffer);
        ui_process_command(buffer);
        goto done;
      }
      i += 1;
    }
  }
  return PT_YIELDED;
done:
error:
  free(buffer);
  buffer = NULL;
  i = 0;
  return PT_YIELDED;
}
  

void UI__setup_std(){
  debug(F("UiStdSetup:"));
  start_thread("*M", system_monitor);
  start_thread("*UI", user_interface);
  
  ui_expose_function("mon", monswitch);
  ui_expose_function("v", cmd_v);
  ui_expose_function("kill", cmd_kill);
  
}



