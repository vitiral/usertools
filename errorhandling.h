/* Copyright (c) 2014, Garrett Berg <garrett@cloudformdesign.com>, cloudformdesign.com
 * This library is released under the FreeBSD License, if you need
 * a copy go to: http://www.freebsd.org/copyright/freebsd-license.html
 * 
 * SUMMARY:
 * Library to help with debugging. See user guide in documentation.
 */


/*
 * Notes:
 * __FILE-__ -> F(__FILE__) ==> 12814 -> 14550 using Flash!!!
 *      The compiler is doing some special stuff for __FILE__
 */
#ifndef __errorhandling_h__
#define __errorhandling_h__

#include <string.h>
#include <errno.h>
#include "logging.h"

#define ERR_NOERR         0 // NoErr -- no error has occured
#define ERR_TIMEOUT       '#' // TimeoutErr
#define ERR_COM           '<' // ComErr
#define ERR_INPUT         '>' // InputErr

#define ERR_TYPE          'T' // TypeErr
#define ERR_VALUE         'V' // ValueErr
#define ERR_ASSERT        'A' // AssertErr
#define ERR_INDEX         'i' // IndexErr
#define ERR_MEMORY        'M' // MemErr
#define ERR_CRITICAL      '!' // CRITErr

#define ERR_CLEARED       '-' // "Cleared Error" used by clrerr_log and is then cleared
#define ERR_NONEW         '*' // NoNew -- error already printed
#define ERR_UNKNOWN       '?' // unknown error

#define EH_config_std            L_config_std()
#define EH_config_soft(soft)     L_config_soft(&soft)

void EH_void_fun(void *input);
void EH_try_end();
void EH_waitc();

void EH_test();
extern uint8_t errprint;
extern char *EH_EMPTY_STR;

uint8_t clrerr();
uint8_t clrerr_if(uint8_t iferr);
uint8_t clrerr_ifn(uint8_t ifnerr);

void seterr(uint8_t error);

#define EH_DW(code) do{code}while(0) //wraps in a do while(0) so that the syntax is correct.

#define raise(E, ...)                           EH_DW(EH_ST_raisem(E, __VA_ARGS__); goto error;)
#define assert(A, ...)                          EH_DW(if(!(A)) {seterr(ERR_ASSERT); log_err(__VA_ARGS__); EH_FLUSH(); goto error;})
#define assert_raise(A, E, ...)                 EH_DW(if(!(A)) {raise((E), __VA_ARGS__);})
#define memcheck(A)                             assert_raise(A, ERR_MEMORY)
#define memclr(M)                               if(M) free(M)

// These functions make the error label unnecessary
#define raise_return(E, ...)                    EH_DW(seterr(E); log_err(); EH_FLUSH(); return __VA_ARGS__;)
#define raisem_return(E, M, ...)                EH_DW(EH_ST_raisem(E, M); return __VA_ARGS__;)
#define assert_return(A, ...)                   EH_DW(if(!(A)){seterr(ERR_ASSERT); log_err(); EH_FLUSH(); return __VA_ARGS__;})
#define assert_raise_return(A, E, ...)          EH_DW(if(!(A)){seterr(E); log_err(); EH_FLUSH(); return __VA_ARGS__;})
#define assert_raisem_return(A, E, M, ...)      EH_DW(if(!(A)){EH_ST_raisem(E, M); return __VA_ARGS__;})
#define memcheck_return(A, ...)                 assert_raise_return(A, ERR_MEMORY, __VA_ARGS__)

#define iferr_return(...)        EH_DW(if(errno) return __VA_ARGS__;)
#define iferr_log_return(...)    EH_DW(if(errno) {log_err(); EH_FLUSH(); return __VA_ARGS__;})
#define iferr_log()              EH_DW(if(errno) {log_err(); EH_FLUSH();})
#define iferr_catch()            EH_DW(if(errno) goto error;)
#define iferr_log_catch()        EH_DW(if(errno) {log_err(); EH_FLUSH(); goto error;})

// Only log at the proper level.
#if LOGLEVEL >= LOG_DEBUG
  void EH_start_debug(char *file, unsigned int line);
  #define sdebug(M) EH_DW(LOG_IFLL(LOG_DEBUG, EH_start_debug(__FILE__, __LINE__); L_print(M); EH_FLUSH();))
  #define cdebug(M) EH_DW(LOG_IFLL(LOG_DEBUG, L_print(M); EH_FLUSH();))
  #define edebug(M) EH_DW(LOG_IFLL(LOG_DEBUG, L_println(M); EH_FLUSH();))
  #define debug(...) EH_DW(LOG_IFLL(LOG_DEBUG, EH_start_debug(__FILE__, __LINE__); L_println(__VA_ARGS__); EH_FLUSH();))
  #define waitc()    EH_DW(EH_start_debug(__FILE__, __LINE__); EH_waitc();)
  #define debug_code(C)   EH_DW(C)
#else
  #define sdebug(M)
  #define cdebug(M)
  #define edebug(M)
  #define debug(...)
  #define waitc() 
  #define debug_code(C)   
#endif

#if LOGLEVEL >= LOG_INFO
void EH_start_info(char *file, unsigned int line);
  #define slog_info(M) EH_DW(LOG_IFLL(LOG_INFO, EH_start_info(__FILE__, __LINE__); L_print(M); EH_FLUSH(); ))
  #define clog_info(M) EH_DW(LOG_IFLL(LOG_INFO, L_print(M); EH_FLUSH();))
  #define elog_info(M) EH_DW(LOG_IFLL(LOG_INFO, L_println(M); EH_FLUSH();))
  
  #define log_info(...) EH_DW(LOG_IFLL(LOG_INFO, EH_start_info(__FILE__, __LINE__); L_println(__VA_ARGS__); EH_FLUSH(); ))
#else
  #define slog_info(M)
  #define clog_info(M)
  #define elog_info(M)
  #define log_info(...) 
#endif

// Used with TRY
#define CATCH_ALL                if(clrerr())
#define CATCH(E)                 if(clrerr_if(E))
#define CATCH_N(E)               if(clrerr_ifn(E))
#define ELSE_CATCH_ALL           else if(clrerr())
#define ELSE_CATCH(E)            else if(clrerr_if(E))
#define ELSE_CATCH_N(E)          else if(clrerr_ifn(E))


#if LOGLEVEL >= LOG_ERROR
  //#define TRY(stuff)               EH_DW(L_silent += 1; EH_void_fun((void *) (stuff)); L_silent -= 1;)
  void EH_log_err(char *file, unsigned int line);
  #define slog_err(M)              EH_DW(LOG_IFLL(LOG_ERROR, EH_log_err(__FILE__, __LINE__); L_print(M); EH_FLUSH(); ))
  #define clog_err(M)              EH_DW(LOG_IFLL(LOG_ERROR, L_print(M); EH_FLUSH();))
  #define elog_err(M)              EH_DW(LOG_IFLL(LOG_ERROR, L_println(M); EH_FLUSH();))
  #define log_err(...)            EH_DW(LOG_IFLL(LOG_ERROR, EH_log_err(__FILE__, __LINE__); L_println(__VA_ARGS__); EH_FLUSH(); ))
  #define EH_ST_raisem(E, ...)    seterr(E); EH_DW(LOG_IFLL(LOG_ERROR, EH_log_err(__FILE__, __LINE__); L_println(__VA_ARGS__); EH_FLUSH(); ))
  #define clrerr_log()            EH_DW(LOG_IFLL(LOG_ERROR, seterr(ERR_CLEARED); log_err(); EH_FLUSH(); clrerr();))
  
  void EH_printerrp();
  void EH_printinfo(char *file, unsigned int line);
  
#else
  #define TRY(stuff)              (stuff)
  #define slog_err(M)
  #define clog_err(M)
  #define elog_err(M)
  #define log_err(...)
  #define EH_ST_raisem(E, ...) seterr(E)
  #define clrerr_log() clrerr()
#endif

#if LOGLEVEL > 0
  #define TRY(stuff)               EH_DW(L_silent += 1; (stuff); L_silent -= 1;)
#endif

#ifdef DEBUG_FLUSH
  extern uint8_t EH_flush;
  #define EH_FLUSH() if(EH_flush){L_flush();}
  #define set_debug_flush(V) EH_flush = (V);
#else
  #define EH_FLUSH()
  #define set_debug_flush(V)
#endif


#endif //file



