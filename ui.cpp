/* Copyright (c) 2014, Garrett Berg <garrett@cloudformdesign.com>, cloudformdesign.com
 * This library is released under the FreeBSD License, if you need
 * a copy go to: http://www.freebsd.org/copyright/freebsd-license.html
 * 
 * SUMMARY:
 * User Interface library to make microcontroller threading integreate into a simple
 *  user interface that allows the calling of functions and threads as well as the 
 *  query of variables.
 * See UserGuide_ui.html for more information.
 */
 
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
#include <avr/interrupt.h>

#define LEDPIN 2


// #####################################################
// ### Globals

#define MAX_STR_LEN 100

uint16_t ui_loop_time = 0;

// #####################################################
// ### Interrupts

void ui_watchdog(){
  Logger.print("[CRITICAL] Timed out:");
  if(th_calling){
    Logger.println(th_calling);
  }
  else if(th_loop_index < 255){
    Logger.println(TH__threads.array[th_loop_index].el.name);
  }
  else {
      Logger.println("Unknown");
  }
  ui_loop_time = millis();
  //asm volatile ("  jmp 0"); 
  wdt_reset();
  wdt_enable(WDTO_250MS);
}

void ui_wdt_enable(void)
{
    //disable interrupts
    cli();
    //reset watchdog
    wdt_reset();
    //set up WDT interrupt
    WDTCSR = (1<<WDCE)|(1<<WDE);
    //Start watchdog timer with 4s prescaller
    WDTCSR = (1<<WDIE)|(1<<WDE)|(1<<WDP3);
    //wdt_enable(WDTO_2S);  // can NOT use this -- must use custom
    //Enable global interrupts
    sei();
}

void ui_watchdog_setup()
{
    ui_wdt_enable();
}

void ui_pat_dog(){
  wdt_reset();
  ui_loop_time = millis(); // pet the custom dog.
}

ISR(WDT_vect) {
   ui_watchdog();
}

// #####################################################
// ### Functions

void __print_row(String *row, uint8_t *col_widths){
  uint8_t si = 0, column = 0, ncolumn = 0;
  while((*row)[si] != 0){
    Logger.write('|');
    Logger.wrote = 0;
    while(column == ncolumn){
      switch((*row)[si]){
      case 0:
        Logger.println();
        return;
      case '\t':
        ncolumn += 1;
        break;
      default:
        Logger.write((*row)[si]);
      }
      si++;
    }
    while(Logger.wrote < col_widths[column]){
      Logger.write(' ');
    }
    column = ncolumn;
  }
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
  sdebug(F("Calling Name:"));
  cdebug(c2); cdebug(':');
  edebug(c);
  call_name(c2, c);
}

void cmd_v(char *input){
  char *word = get_word(input);
  iferr_log_return();
  print_variable(word);
}

void cmd_kill(char *input){
  input = pass_ws(input);
  char *wordend = get_word_end(input);
  *wordend = 0;
  uint16_t v;
  char **tailptr;
  iferr_log_return();
  sdebug(F("killing:")); edebug(input);
  thread *th = TH_get_thread(input);
  if(th) {
    debug(th->el.name);
    kill_thread(th);
  }
  else{
    raise_return(ERR_INPUT);
  }
  /*
  else {
    v = strtol(input, tailptr, 10);
    debug(input);
    debug((unsigned short)(*tailptr), HEX);
    debug((unsigned long)input, HEX);
    debug((unsigned long)*tailptr, HEX);
    debug((unsigned long)**tailptr, HEX);
    debug(tailptr == NULL);
    sdebug(*tailptr == wordend); cdebug(F(" kill value ")); edebug(v);
    assert_raise_return(*tailptr == wordend, ERR_INPUT);
    //assert_raise_return(tailptr != NULL, ERR_INPUT);   //UNDOCUMENTED: apparently the value can be null! fun!
    //assert_raise_return(**tailptr == 0, ERR_INPUT);
    kill_thread(v);
  }
  */
  Logger.print(F("Killed:"));
  Logger.println(input);

}

void _print_monitor(uint16_t execution_time){
  Logger.print(F("Total Time: "));
  Logger.print(execution_time);
  Logger.print(F("  Free Memory:"));
  Logger.println(freeMemory());
  
  
  char nameray[] = "Ind\tName\tt exec[ms]\tLine"; // also used for storing flash names
  thread *th;
  uint8_t column_widths[] = {4, 12, 12, 12};
  
  String myStr;
  
  print_row(String(nameray), column_widths);
  for(int i = 0; i < TH__threads.index; i++){
    th = &TH__threads.array[i];
    if(th->pt.lc >= PT_KILL_VALUE)continue;
    flash_to_str(th->el.name, nameray);
    myStr = String(i)                                               + String('\t') + 
      String(nameray)                                               + String('\t') + 
      String(th->time / 100) + String('.') + String(th->time %100)  + String('\t') +
      String((unsigned int)th->pt.lc);
    print_row(myStr, column_widths);
    ui_pat_dog();
    th->time = 0; // reset time
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
  Logger.repeat('*', 5);
  Logger.println(F("Threads"));
  for(i = 0; i < TH__threads.index; i++){
    Logger.print(i);
    Logger.write('\t');
    Logger.println(TH__threads.array[i].el.name);
  }
  
  Logger.repeat('*', 5);
  Logger.println(F("\nVars"));
  for(i = 0; i < TH__variables.index; i++){
    Logger.println(TH__variables.array[i].el.name);
  }
  
  Logger.repeat('*', 5);
  Logger.println(F("\nFuncs"));
  for(i = 0; i < TH__functions.index; i++){
    Logger.println(TH__functions.array[i].el.name);
  }
}

uint8_t print_variable(char *name){
  TH_variable *var;
  int8_t n;
  uint8_t i;
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

void ui_std_greeting(){
  Logger.println(F("!!!Make sure you are sending NL + CR\n?=help\n"));
}

void UI__setup_std(){
  debug(F("UiStdSetup:"));
  start_thread("*UI", user_interface);  // user interface. REQUIRED
  
  expose_thread("mon", system_monitor); // system monitor
  
  expose_function("v", cmd_v);
  expose_function("?", print_options);
  expose_function("kill", cmd_kill);
  
  
  ui_watchdog_setup();
}

void ui_loop(){
  ui_pat_dog();
  
  Logger.wrote = false;
  thread_loop();
  if(Logger.wrote){
    Logger.repeat('#', 5);
    Logger.println();
  }
}



