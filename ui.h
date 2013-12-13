
#ifndef ui_h
#define ui_h

#include "usertools.h"
#include <Arduino.h>
#include "pt.h"
#include "threading.h"

// Helpful for String Parsing
#define get_word(C) _get_word(&(C))
#define get_int(C) _get_int(&(C))

#define ui_setup_std(...) do{thread_setup(__VA_ARGS__); \
                              start_thread("*M", system_monitor); \
                              start_thread("*UI", user_interface); \
                              ui_expose_function("mon", monswitch); \
                              }while(0)

extern char UI_CMD_END_CHAR;
extern char UI_CMD_PEND_CHAR;  // may be right before the end.
extern char *UI_TABLE_SEP;

char *_get_word(char **c);
long int _get_int(char **c);

// #####################################################
// ### Functions
uint8_t print_variable(char *name);
uint8_t call_function(char *name, char *input);
#define ui_loop() thread_loop()

uint8_t system_monitor(pthread *pt, char *input);
uint8_t monswitch(pthread *pt, char *input);


// #####################################################
// ### Other Module Functions
//void ui_process_command(char *c);
void ui_interface();

#endif

