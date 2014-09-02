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
#include "logging.h"
#include "ReMem.h"

#include "ptsmall.h"

extern ReMem PT__RM;

#define setup_pt(M)   PT__RM.init(M)

// commented = not supported, but may in future
enum vtype{
  vt_minint,
  // Integers
  //vt_int8,
  vt_uint8,

  vt_int16,
  vt_uint16,

  //vt_int32,
  //vt_uint32,

  vt_uint64,
  vt_maxint,

  //Float
  vt_float64_t,

  vt_uint8ray,  // 8 bit uint arrays
  vt_int16ray,  // 16 bit int arrays

  vt_str,       // storing character arrays

  vt_pt,        // storing protothreads

  vt_errtype = 0x3F
};

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
  uint8_t get_type_type(ptindex index, uint8_t type);

  void *put_data(void *putdata, uint8_t type);
  void *put_data(void *putdata, uint8_t type, uint16_t len);
  void destroy_data(PT_data *pd, PT_data *prev);
  int32_t get_int_type(ptindex index, uint8_t type);
  int32_t get_int(PT_data_int32 *pint);

  PT_data *get_temp(uint8_t type);

  char *get_str_type(ptindex index, uint8_t type);

  void clear_type(uint8_t type, ptindex index);

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
  void del_input(uint8_t index);
  void clear_input();

  int32_t get_int_input(ptindex index);
  char *get_str_input(ptindex index);
  uint8_t get_type_input(ptindex index);

  // Output
  void *put_output(uint8_t output);
  void *put_output(int16_t output);
  void *put_output(uint16_t output);
  void *put_output(int32_t output);
  void *put_output(char *output, uint16_t len);
  void *put_output(char *output);
  void del_output(uint8_t index);
  void clear_output();

  int32_t get_int_output(ptindex index);
  char *get_str_output(ptindex index);
  uint8_t get_type_output(ptindex index);

  // General Destructors
  void clear_data();

  void print();
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

void transfer_inputs(pthread *from, pthread *to);

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
  (pt)->put_temp((uint16_t)millis());                                 \
  PT_WAIT_UNTIL(pt, ((uint16_t)millis()) - (uint16_t)(pt)->get_int_temp() \
    > ms);                                                            \
  } while(0)


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
    (pt)->put_temp_pt();                                            \
    PT_WAIT_THREAD(pt, thread((pt)->get_pt_temp(), __VA_ARGS__));		\
    (pt)->clear_temp();                                             \
  } while(0)

/** @} */


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

#endif /* __PT_H__ */

/** @} */
