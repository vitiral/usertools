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
 
#ifndef ui_h
#define ui_h

#include "usertools.h"
#include <Arduino.h>
#include "pt.h"
#include "threading.h"

// Helpful for String Parsing
#define get_word(C) _get_word(&(C))
#define get_int(C) _get_int(&(C))

#define UI__MIN_F 3
#define UI__MIN_T 2
#define ui_setup_std(V, F, T) do{thread_setup(V, (F) + UI__MIN_F, (T) + UI__MIN_T); UI__setup_std();}while(0)

void UI__setup_std();
#define UI_CMD_END_CHAR 0x0A
#define UI_CMD_PEND_CHAR 0x0D  // may be right before the end.

#define UI_TABLE_SEP F(" \t ")

char *_get_word(char **c);
long int _get_int(char **c);

// #####################################################
// ### Functions
uint8_t print_variable(char *name);

void ui_watchdog();
void ui_pat_dog();
void system_monitor(char *input);
void monswitch(char *input);
void print_options(char *input);
void ui_loop();
void ui_std_greeting();
#define print_row(S, C) __print_row(&(S), C)
void __print_row(String *row, uint8_t *col_widths);


// #####################################################
// ### Other Module Functions
//void ui_process_command(char *c);
void user_interface();

#endif

