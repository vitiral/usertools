#ifndef threading_h
#define threading_h

#include "usertools.h"
#include "pt.h"

// #####################################################
// ### Struct Declaration

typedef uint8_t (*TH_funptr)(struct pt *pt, char *input);

// 3 bytes
typedef struct TH_element {
  const __FlashStringHelper   *name;
  uint8_t                     name_len;
};

// 6 bytes
typedef struct TH_variable {
  TH_element  el;
  void        *vptr;
  uint8_t     size;
};

// 12 bytes 
typedef struct thread{
  TH_element     el;
  TH_funptr      fptr;
  struct pt      pt;
  uint16_t time;  // stores execution time in 10us increments
};

typedef struct TH_Variables{
  TH_variable *array;
  uint8_t len;
  uint16_t index;
};

typedef struct TH_ThreadArray{
  struct thread *array;
  uint8_t len;
  uint16_t index;
};
  
class TH_thread_instance
{
public:
  thread             *th;
  TH_thread_instance(thread *th_in){
    th = th_in;
  }
  TH_thread_instance(){
    th = NULL;
  }
};

// #####################################################
// ### Intended Exported Functions

// Setup Macros

#define thread_setup(V, T) do{static TH_variable UI__variable_array[V]; \
      static uint8_t UI__variable_len = V;                              \
      UI__set_variable_array(UI__variable_array, UI__variable_len);     \
                                                                        \
      static thread UI__function_array[T];                              \
      static uint8_t UI__function_len = T;                              \
      UI__set_function_array(UI__function_array, UI__function_len);     \
    }while(0)
                                       
//#define thread_function(name, func)  do{static thread name; schedule_function(name, func)}while(0) 

// Expose Macros
#define ui_expose_variable(name, var) UI__expose_variable(F(name), (void *)&(var), sizeof(var)) 
#define ui_expose_function(name, FUN)         UI__expose_function(F(name), FUN)

#define start_thread(name, func) schedule_function(F(name), func)

uint8_t thread_loop();

void kill_thread(thread *th);


// #####################################################
// ### Macro Helpers

void UI__set_variable_array(TH_variable *vray, uint16_t len);
void UI__expose_variable(const __FlashStringHelper *name, void *varptr, uint8_t varsize);

void UI__set_function_array(thread *fray, uint16_t len);
thread *UI__expose_function(const __FlashStringHelper *name, TH_funptr fptr);



// #####################################################
// ### User Functions

thread *schedule_function(const __FlashStringHelper *name, TH_funptr fun);
uint8_t call_thread(char *name, char *input);
uint8_t ui_loop();
uint8_t restart_thread(thread *th);


// #####################################################
// ### Package Access

void TH__set_innactive(thread *f);
extern TH_Variables TH__variables;
extern TH_ThreadArray TH__th_array;
TH_variable *TH_get_variable(char *name);
thread *TH_get_thread(char *name);

#endif
