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

typedef uint8_t (*TH_funptr)(struct pt *pt, char *input);

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
  TH_funptr                   fptr;
  struct pt                   pt;
};

typedef struct TH_Variables{
  ui_variable *array;
  uint8_t len;
  uint16_t index;
};

typedef struct TH_Functions{
  struct ui_function *array;
  uint8_t len;
  uint16_t index;
};
  
class TH_thread_instance
{
public:
  ui_function             *function;
  TH_thread_instance(ui_function *func){
    function = func;
  }
  TH_thread_instance(){
    function = NULL;
  }
};


// #####################################################
// ### Macro Helpers

void UI__set_variable_array(ui_variable *vray, uint16_t len);
void UI__expose_variable(const __FlashStringHelper *name, void *varptr, uint8_t varsize);

void UI__set_function_array(ui_function *fray, uint16_t len);
ui_function *UI__expose_function(const __FlashStringHelper *name, TH_funptr fptr);


ui_function *schedule_function(const __FlashStringHelper *name, TH_funptr fun);
uint8_t call_function(char *name, char *input);
uint8_t ui_loop();


// #####################################################
// ### Package Access

void TH__set_innactive(ui_function *f);
extern TH_Variables TH__variables;
extern TH_Functions TH__functions;

//extern LinkedList<TH_thread_instance> TH__threads;

#endif
