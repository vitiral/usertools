
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

#include <avr/wdt.h>
#include <TInterrupts.h>

#include <avr/interrupt.h>

#define LEDPIN 2


// #####################################################
// ### Globals

#define MAX_STR_LEN 100
uint16_t ui_loop_time = 0;

// #####################################################
// ### Interrupts

void ui_watchdog(){
  if((uint16_t)millis() - ui_loop_time > 1000){
    Logger.print("[CRITICAL] Timed out:");
    if(th_calling){
      Logger.println(th_calling);
    }
    else if(th_loop_index < 255){
      Logger.println(TH__threads.array[th_loop_index].el.name);
    }
    else Logger.println("Unknown");
    Logger.flush();
    //asm volatile ("  jmp 0"); 
    wdt_enable(WDTO_15MS);
    while(1);      // wait for reset
  }
}

void ui_timer1_setup()
{
    debug("setting up timer");
    pinMode(LEDPIN, OUTPUT);
    // initialize Timer1
    cli();         // disable global interrupts
    TCCR1A = 0;    // set entire TCCR1A register to 0
    TCCR1B = 0;
    // enable Timer1 overflow interrupt:
    TIMSK1 = (1 << TOIE1);
    // Set CS12 bit so timer runs at clock speed/256:
    //TCCR1B |= (1 << CS12);
    TCCR1B |= (1 << CS10);
    
    // enable global interrupts:
    sei();
}


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
  c2 = get_word(c);
  iferr_log_return();
  assert_return(c); assert_return(c2);
  sdebug(F("Calling Function:"));
  cdebug(c2); cdebug(':');
  edebug(c);
  th_call_name(c2, c);
}

void cmd_v(char *input){
  char *word = get_word(input);
  iferr_log_return();
  print_variable(word);
}

void cmd_kill(char *input){
  char *word = get_word(input);
  uint16_t v;
  char **tailptr;
  iferr_log_return();
  sdebug(F("killing:")); edebug(word);
  thread *th = TH_get_thread(word);
  if(th) {
    debug(th->el.name);
    kill_thread(th);
  }
  else {
    v = strtol(word, tailptr, 0);
    assert_raise_return(word != *tailptr, ERR_INPUT);
    kill_thread(v);
  }
  Logger.print(F("Killed:"));
  Logger.println(word);

}

void _print_monitor(uint16_t execution_time){
  Logger.print(F("Total Time: "));
  Logger.print(execution_time);
  Logger.print(F(" \t\tFree Memory:"));
  Logger.println(freeMemory());

  Logger.println(F("Name\t\tExTime\t\tLine"));
  int16_t i = 0;
  thread *th;
  while(i < TH__threads.index){
    th = &TH__threads.array[i];
    if(th->pt.lc >= PT_KILL_VALUE) continue;
    Logger.print(i);
    Logger.write(' ');
    Logger.print(th->el.name);
    Logger.print(UI_TABLE_SEP);
    Logger.print(th->time / 100);
    Logger.write('.');
    Logger.print(th->time % 100);
    Logger.print(UI_TABLE_SEP);
    Logger.println((unsigned int)th->pt.lc);
    //Logger.print(UI_TABLE_SEP);
    //Logger.println(th->pt.error);
    th->time = 0; // reset time
    i++;
  }
}

uint8_t system_monitor(pthread *pt, char *input){
  static uint16_t time;
  PT_BEGIN(pt);
  while(true){
    PT_WAIT_MS(pt, time, 5000);
    _print_monitor((uint16_t)millis() - time);
  }
  PT_END(pt);
}

void print_options(char *input){
  uint8_t i = 0;
  Logger.println(F("Threads"));
  for(i = 0; i < TH__threads.index; i++){
    Logger.print(i);
    Logger.write('\t');
    Logger.println(TH__threads.array[i].el.name);
  }
  
  Logger.println(F("\nVars"));
  for(i = 0; i < TH__variables.index; i++){
    Logger.println(TH__variables.array[i].el.name);
  }
  
  Logger.println(F("\nFuncs"));
  for(i = 0; i < TH__functions.index; i++){
    Logger.println(TH__functions.array[i].el.name);
  }
}

uint8_t print_variable(char *name){
  TH_variable *var;
  int8_t n;
  uint8_t i;
  uint8_t name_len = strlen(name);
  var = TH_get_variable(name);
  assert_raise_return(var, ERR_INPUT, false);
  Logger.print(F("v=x"));
  for(n = var->size - 1; n >= 0; n--){
    uint8_t *chptr = (uint8_t*)(var->vptr);  // works
    if(chptr[n] < 0x10) Logger.write('0');
    Logger.print(chptr[n], HEX);
  }
  Logger.println();
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
  
  if(Logger.available()){
    while(Logger.available()){
      c = Logger.read();
      buffer[i] = c;
      if(i > MAX_STR_LEN){
        while(Logger.available()) Logger.read();
        raise(ERR_COM, F("Size"));
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
  start_thread("*UI", user_interface);  // user interface. REQUIRED
  
  ui_expose_thread("mon", system_monitor); // system monitor
  
  ui_expose_function("v", cmd_v);
  ui_expose_function("?", print_options);
  ui_expose_function("kill", cmd_kill);
  
  ui_timer1_setup();
  //wdt_enable(WDTO_4S);  //reset after 4S if no pat received
  
}

void ui_loop(){
  ui_loop_time = millis();
  
  thread_loop();
}



