
#include <Arduino.h>
#include "pt.h"
#include "threading.h"

// Helpful for String Parsing
#define get_word(C) _get_word(&(C))
#define get_int(C) _get_int(&(C))


char *_get_word(char **c);
long int _get_int(char **c);

// #####################################################
// ### Functions
uint8_t print_variable(char *name);
uint8_t call_function(char *name, char *input);
uint8_t ui_loop();

