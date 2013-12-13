#ifndef threading_h
#define threading_h

#include "pt.h"

// #####################################################
// ### Intended Exported Functions
typedef struct pt thread;

// Setup Macros
void ui_setup_std();
#define ui_declare_variables(len) ui_variable UI__variable_array[len]; uint8_t UI__variable_len = len
#define ui_setup_variables()      UI__set_variable_array(UI__variable_array, UI__variable_len)
#define ui_declare_functions(len) ui_function UI__function_array[len]; uint8_t UI__function_len = len
#define ui_setup_functions()      UI__set_function_array(UI__function_array, UI__function_len)
//#define thread_function(name, func)  do{static ui_function name; schedule_function(name, func)}while(0) 

// Expose Macros
#define ui_expose_variable(name, var) UI__expose_variable(F(name), (void *)&(var), sizeof(var)) 
#define ui_expose_function(name, FUN)         UI__expose_function(F(name), FUN)


// #####################################################
// ### Struct Declaration

typedef uint8_t (*ui_funptr)(struct pt *pt, char *input);

typedef struct ui_element {
  const __FlashStringHelper   *name;
  uint8_t                     name_len;
};

typedef struct ui_variable {
  ui_element                  el;
  void                        *vptr;
  uint8_t                     size;
};

typedef struct ui_function{
  ui_element                  el;
  ui_funptr                   fptr;
  struct pt                   pt;
};

// #####################################################
// ### Macro Helpers

void UI__set_variable_array(ui_variable *vray, uint16_t len);
//void UI__expose_variable(const __FlashStringHelper *name, void *varptr, uint8_t varsize);
void UI__expose_variable(const __FlashStringHelper *name, void *varptr, uint8_t varsize);

void UI__set_function_array(ui_function *fray, uint16_t len);
ui_function *UI__expose_function(const __FlashStringHelper *name, ui_funptr fptr);

// #####################################################
// ### Functions
ui_function *schedule_function(const __FlashStringHelper *name, ui_funptr fun);


#endif
