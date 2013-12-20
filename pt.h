/* 
 * Copyrigt (C) 2014 - 2015, Garrett Berg cloudformdesign.com
 * Copyright (c) 2004-2005, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: pt.h,v 1.7 2006/10/02 07:52:56 adam Exp $
 */

/**
 * \addtogroup pt
 * @{
 */

/**
 * \file
 * Protothreads implementation.
 * \author
 * Adam Dunkels <adam@sics.se>
 *
 */

#ifndef __PT_H__
#define __PT_H__

#include "usertools.h"
#include "errorhandling.h"

#include "lc.h"

#define PT_KILL_VALUE LC_KILL_VALUE
#define PT_INNACTIVE  LC_KILL_VALUE + 1

#define PT_DATA_BYTES 3

typedef uint8_t ptindex;
struct PT_data;
class pthread;

// 3 bytes
struct PT_data_base {
  struct PT_data *next;
  uint8_t type;
};

struct PT_data {
  struct PT_data_base b;
};

struct PT_data_int8{
  struct PT_data_base b;
  int8_t data;
};

struct PT_data_uint8{
  struct PT_data_base b;
  uint8_t data;
};

struct PT_data_int16{
  struct PT_data_base b;
  int16_t data;
};

struct PT_data_int32{
  struct PT_data_base b;
  int32_t data;
};

struct PT_data_str{
  struct PT_data_base b;
  char data[];
};

class pthread
{
private:
  PT_data *get_end();
  PT_data *get_type(ptindex index, uint8_t type);
  
  void *put_data(void *putdata, uint8_t type);
  void *put_data(void *putdata, uint8_t type, uint16_t len);
  void destroy_data(PT_data *pd, PT_data *prev);
  int32_t get_int_type(ptindex index, uint8_t type);
  int32_t get_int(PT_data_int32 *pint);
  
  PT_data *get_temp(uint8_t type);
  
  char *get_str_type(ptindex index, uint8_t type);
  
public:
  lc_t lc;
  PT_data *data;
  // Constructors
  pthread();
  ~pthread();
  
  // Temp
  void *put_temp(uint16_t input);
  void *put_temp_pt();
  
  uint16_t get_int_temp();
  pthread *get_pt_temp();
  void clear_temp();
  
  // Input
  void *put_input(uint8_t input);
  void *put_input(int16_t input);
  void *put_input(uint16_t input);
  void *put_input(int32_t input);
  void *put_input(char *input, uint16_t len);
  void *put_input(char *input);
  void clear_input();
  
  int32_t get_int_input(ptindex index);
  char *get_str_input(ptindex index);
  
  // Output
  void *put_output(uint8_t output);
  void *put_output(int16_t output);
  void *put_output(uint16_t output);
  void *put_output(int32_t output);
  void *put_output(char *output, uint16_t len);
  void *put_output(char *output);
  void clear_output();
  
  int32_t get_int_output(ptindex index);
  char *get_str_output(ptindex index);
  
  // General Destructors
  void clear_data();
  void clear_type(uint8_t type);
};

struct PTsmall {
  lc_t lc;
};

struct PTnorm {
  lc_t lc;
  PT_data *data;
};

// data for storing another pthread
// stored only in temp.
struct PT_data_pt{
  struct PT_data_base b;
  pthread data;
};

enum PTVALUES {
  PT_WAITING    ,
  PT_YIELDED    ,
  PT_ERROR      ,
  PT_EXITED     ,
  PT_ENDED      ,
  PT_KILLED     ,
  PT_CRITICAL   
};

/**
 * \name Initialization
 * @{
 */

/**
 * Initialize a protothread.
 *
 * Initializes a protothread. Initialization must be done prior to
 * starting to execute the protothread.
 *
 * \param pt A pointer to the protothread control structure.
 *
 * \sa PT_SPAWN()
 *
 * \hideinitializer
 */
#define PT_INIT(pt)   do{LC_INIT((pt)->lc); (pt)->clear_data();} while(0)

/** @} */

/**
 * \name Declaration and definition
 * @{
 */

/**
 * Declaration of a protothread.
 *
 * This macro is used to declare a protothread. All protothreads must
 * be declared with this macro.
 *
 * \param name_args The name and arguments of the C function
 * implementing the protothread.
 *
 * \hideinitializer
 */
#define PT_THREAD uint8_t

/**
 * \name Declaration and definition
 * @{
 */

/**
 * Declare the start of a protothread inside the C function
 * implementing the protothread.
 *
 * This macro is used to declare the starting point of a
 * protothread. It should be placed at the start of the function in
 * which the protothread runs. All C statements above the PT_BEGIN()
 * invokation will be executed each time the protothread is scheduled.
 *
 * \param pt A pointer to the protothread control structure.
 *
 * \hideinitializer
 */
#define PT_BEGIN(pt) { uint8_t PT_YIELD_FLAG = 1; LC_RESUME((pt)->lc)

/**
 * Declare the end of a protothread.
 *
 * This macro is used for declaring that a protothread ends. It must
 * always be used together with a matching PT_BEGIN() macro.
 *
 * \param pt A pointer to the protothread control structure.
 *
 * \hideinitializer
 */
#define PT_END(pt) LC_END((pt)->lc); PT_YIELD_FLAG = 0; \
                   PT_INIT(pt); return PT_ENDED; }

/** @} */

/**
 * \name Blocked wait
 * @{
 */

/**
 * Block and wait until condition is true.
 *
 * This macro blocks the protothread until the specified condition is
 * true.
 *
 * \param pt A pointer to the protothread control structure.
 * \param condition The condition.
 *
 * \hideinitializer
 */
#define PT_WAIT_UNTIL(pt, condition)	        \
  do {						\
    LC_SET((pt)->lc);				\
    if(!(condition)) {				\
      return PT_WAITING;			\
    }						\
  } while(0) 

/**
 * Block and wait until time in milliseconds has passed
 *
 * This macro blocks the protothread until at least the specified
 * time in ms has passed.
 *
 * \param pt A pointer to the protothread control structure.
 * \param ms The number of milliseconds to wait.
 *
 * \hideinitializer
 */
 
 #define PT_WAIT_MS(pt, ms)                                           \
  do {                                                                \
  (pt)->clear_temp();                                                 \
  (pt)->put_temp((uint16_t)millis());                                 \
  iferr_return(PT_CRITICAL);   /*mem error*/                          \
  PT_WAIT_UNTIL(pt, ((uint16_t)millis()) - (uint16_t)(pt)->get_int_temp() \
    > ms);                                                            \
  (pt)->clear_temp();                                                 \
  } while(0)

/**
 * Block and wait while condition is true.
 *
 * This function blocks and waits while condition is true. See
 * PT_WAIT_UNTIL().
 *
 * \param pt A pointer to the protothread control structure.
 * \param cond The condition.
 *
 * \hideinitializer
 */
#define PT_WAIT_WHILE(pt, cond)  PT_WAIT_UNTIL((pt), !(cond))

/** @} */

/**
 * \name Hierarchical protothreads
 * @{
 */

/**
 * Block and wait until a child protothread completes.
 *
 * This macro schedules a child protothread. The current protothread
 * will block until the child protothread completes.
 *
 * \note The child protothread must be manually initialized with the
 * PT_INIT() function before this function is used.
 *
 * \param pt A pointer to the protothread control structure.
 * \param thread The child protothread with arguments
 *
 * \sa PT_SPAWN()
 *
 * \hideinitializer
 */
#define PT_WAIT_THREAD(pt, thread) PT_WAIT_WHILE((pt), PT_SCHEDULE(thread))

/**
 * Spawn a child protothread and wait until it exits.
 *
 * This macro spawns a child protothread and waits until it exits. The
 * macro can only be used within a protothread.
 *
 * \param pt A pointer to the protothread control structure.
 * \param thread The child protothread function (not called)
 * \param ... thread will be called as "thread(spawned_pt, additional_args)"
 * 
 * The temporary protothread can be gotten from pt.get_pt_temp()
 * From that you can get the outputs 
 *    Ex: myint = pt.get_pt_temp()->get_int_output(index)
 * You should get the inputs and outputs you need and then call 
 *  "pt.clear_temp()" to help conserve memory.
 * 
 * Note that this function automatically clears temp at the beggining
 *   of it's call.
 *
 * \hideinitializer
 */
#define PT_SPAWN(pt, thread, ...)		                              \
  do {						                                                \
    (pt)->clear_temp();                                             \
    (pt)->put_temp_pt();                                            \
    PT_WAIT_THREAD(pt, thread((pt)->get_pt_temp(), __VA_ARGS__));		\
  } while(0)

/** @} */

/**
 * \name Exiting and restarting
 * @{
 */

/**
 * Restart the protothread.
 *
 * This macro will block and cause the running protothread to restart
 * its execution at the place of the PT_BEGIN() call.
 *
 * \param pt A pointer to the protothread control structure.
 *
 * \hideinitializer
 */
#define PT_RESTART(pt)				\
  do {						\
    PT_INIT(pt);				\
    return PT_WAITING;			\
  } while(0)

/**
 * Exit the protothread.
 *
 * This macro causes the protothread to exit. If the protothread was
 * spawned by another protothread, the parent protothread will become
 * unblocked and can continue to run.
 *
 * \param pt A pointer to the protothread control structure.
 *
 * \hideinitializer
 */
#define PT_EXIT(pt)				\
  do {						\
    PT_INIT(pt);				\
    return PT_EXITED;			\
  } while(0)

/** @} */

/**
 * \name Calling a protothread
 * @{
 */

/**
 * Schedule a protothread.
 *
 * This function shedules a protothread. The return value of the
 * function is non-zero if the protothread is running or zero if the
 * protothread has exited.
 *
 * \param f The call to the C function implementing the protothread to
 * be scheduled
 *
 * \hideinitializer
 */
#define PT_SCHEDULE(f) ((f) < PT_EXITED)

/** @} */

/**
 * \name Yielding from a protothread
 * @{
 */

/**
 * Yield from the current protothread.
 *
 * This function will yield the protothread, thereby allowing other
 * processing to take place in the system.
 *
 * \param pt A pointer to the protothread control structure.
 *
 * \hideinitializer
 */
#define PT_YIELD(pt)				\
  do {						\
    PT_YIELD_FLAG = 0;				\
    LC_SET((pt)->lc);				\
    if(PT_YIELD_FLAG == 0) {			\
      return PT_YIELDED;			\
    }						\
  } while(0)

    
/**
 * Yield from the current protothread, clearing output and putting the 
 * specifid value into it.
 * 
 * output can be obtained from (for an integer):
 *  Ex: mmyint = pt->get_int_output(0);
 *
 * This function will yield the protothread, thereby allowing other
 * processing to take place in the system.
 *
 * \param pt A pointer to the protothread control structure.
 * \param value The value you want to be put into the buffer
 * 
 * Note that this function only clears the output at the beggining (not the end)
 * \hideinitializer
 */
#define PT_YIELD_VALUE(pt, value)               \
  do{                                           \
    (pt)->clear_output();                       \
    (pt)->put_output(value);                    \
    iferr_return(PT_CRITICAL); /*mem error*/    \
    PT_YIELD(pt);                               \
  }

/**
 * \brief      Yield from the protothread until a condition occurs.
 * \param pt   A pointer to the protothread control structure.
 * \param cond The condition.
 *
 *             This function will yield the protothread, until the
 *             specified condition evaluates to true.
 *
 *
 * \hideinitializer
 */
#define PT_YIELD_UNTIL(pt, cond)		\
  do {						\
    PT_YIELD_FLAG = 0;				\
    LC_SET((pt)->lc);				\
    if((PT_YIELD_FLAG == 0) || !(cond)) {	\
      return PT_YIELDED;			\
    }						\
  } while(0)

/**
 * Mark where to go if the thread is "killed" by an outside process
 *
 * This allows the thread to do final closing operation if killed.
 * Note: the thread will always enounter this if it does not exit
 *  before hand.
 * Note: it is proper to return PT_KILLED
 *
 * \hideinitializer 
 */

#define PT_KILL(s) LC_KILL(s)

#define PT_KILLED(s) do{PT_INIT(s); return PT_KILLED;} while(0) 

/** @} */

#endif /* __PT_H__ */

/** @} */
