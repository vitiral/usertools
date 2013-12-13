
#include "usertools.h"

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "LinkedList.h"

#include <SoftwareSerial.h>
#include "errorhandling.h"

#include "ui.h"
#include "threading.h"
#include "strtools.h"

#define MAX_STR_LEN 100
#define MAX_ARRAY_LEN 256

extern LinkedList<TH_thread_instance> TH__threads;

//char *LINE_FEED = "\x0D\x0A";
char UI_CMD_END_CHAR = 0x0A;
char UI_CMD_PEND_CHAR = 0x0D;  // may be right before the end.
char *UI_TABLE_SEP = " \t ";

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

void ui_process_command(char *c){
  char *c2;
  char *word;
  debug("Parse CMD");
  //debug(*c);
  c = pass_ws(c);
  //debug(*c);
  c2 = get_word(c);
  iferr_log_return();
  debug(String("Calling Function:") + String(c2) + String(":") + c);
  call_thread(c2, c);
}

uint8_t cmd_v(pthread *pt, char *input){
  debug("cmd 'v'");
  char *word = get_word(input);
  iferr_log_return(PT_ENDED);
  debug(String("var:") + word);
  print_variable(word);
  return PT_ENDED;
}

uint8_t cmd_kill(pthread *pt, char *input){
  char *word = get_word(input);
  iferr_log_return(PT_ENDED);
  debug(String("killing:") + word);
  thread *th = TH_get_thread(word);
  debug(th->el.name);
  assert_raise_return(th, ERR_INPUT, PT_ENDED);
  kill_thread(th);
  Serial.print("Killed:");
  Serial.println(word);
  return PT_ENDED;
}

struct ui_buffer{
  char buffer[MAX_STR_LEN + 2];
  uint8_t i;
} 
ui_buffer;

void clr_ui_buff(){
  ui_buffer.i = 0;
  ui_buffer.buffer[0] = 0;
}

void UI__setup_ui(){
  ui_buffer.i = 0;
  ui_buffer.buffer[0] = 0;
}

void _print_monitor(uint32_t execution_time){
  debug("printing Threads");
  Serial.print(F("Total Time Running Threads: "));
  Serial.println(execution_time);
  Serial.println(F("Name\t\tExTime\t\tLine\t\tError"));
  uint8_t i = 0;
  thread *th;
  while(i < TH__threads.size()){
    th = TH__threads.get(i).th;
    Serial.print(th->el.name);
    Serial.print(UI_TABLE_SEP);
    Serial.print(th->time / 100);
    Serial.write('.');
    Serial.print(th->time % 100);
    Serial.print(UI_TABLE_SEP);
    Serial.print((unsigned int)th->pt.lc);
    Serial.print(UI_TABLE_SEP);
    Serial.println(th->pt.error);
    
    th->time = 0; // reset time
    i++;
  }
  
}

uint8_t print_periodically = false; 

uint8_t monswitch(pthread *pt, char *input){
  char *word = get_word(input);
  if(*word == 0) return PT_ENDED;
  iferr_log_return(PT_ENDED);
  if(cmp_str_flash(word, F("on"))) print_periodically = true;
  if(cmp_str_flash(word, F("off"))) print_periodically = false;
  raisem_return(ERR_INPUT, input, PT_ENDED);
  return PT_ENDED;
}

uint8_t system_monitor(pthread *pt, char *input){
  static uint32_t execution_time; // execution time in ms
  
  PT_BEGIN(pt);
  execution_time = millis();
  while(true){
    PT_WAIT_MS(pt, 5000);
    if(print_periodically) _print_monitor((uint32_t)millis() - execution_time);
    execution_time = millis();
  }
  PT_END(pt);
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
        ui_process_command(ui_buffer.buffer);
        if(Serial.available()) {
          debug(Serial.peek(), HEX);
        }
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
  return PT_YIELDED;
}
  

void UI__setup_std(uint8_t V, uint8_t F){
  start_thread("*M", system_monitor);
  start_thread("*UI", user_interface);
  
  ui_expose_function("mon", monswitch);
  ui_expose_function("v", cmd_v);
  ui_expose_function("kill", cmd_kill);
  
}



