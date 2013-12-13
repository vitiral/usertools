/*
 LICENSE: This code is released under the GNU GPL v3 or greater. For more info see:
 <http://www.gnu.org/licenses/gpl.html>
 Copyright: Garrett Berg cloudformdesign.com garrett@cloudformdesign.com
 Loosly based on code from the fantastic "Learn C the Hard Way",  Zed A. Shaw
 http://c.learncodethehardway.org/book/ex20.html
*/

#ifndef __errorhandling_h__
#define __errorhandling_h__

#define ERR_NOERR         0 // NoErr -- no error has occured
#define ERR_BASE          1 // BaseErr -- general error
#define ERR_TIMEOUT       2 // TimeoutErr
#define ERR_SERIAL        3 // SerialErr
#define ERR_SPI           4 // Spierr
#define ERR_I2C           5 // I2cErr
#define ERR_COMMUNICATION 6 // ComErr
#define ERR_CONFIG        7 // ConfigErr
#define ERR_PIN           8 // PinErr
#define ERR_INPUT         9 // InputErr

#define ERR_TYPE          50 // TypeErr
#define ERR_VALUE         51 // ValueErr
#define ERR_ASSERT        52 // AssertErr
#define ERR_TESTFAIL      53 // TestFail
#define ERR_PTR           54 // PtrErr
#define ERR_INDEX         55 // IndexErr
#define ERR_SIZE          56 // SizeErr
#define ERR_THREAD        57 // ThreadErr

#define ERR_CLEARED       252 // "Cleared Error" used by clrerr_log and is then cleared
#define ERR_NONEW         253 // NoNew -- error already printed
#define ERR_EMPTY         254 // nothing printed, still an error
#define ERR_UNKNOWN       255 // unknown error

#include <SoftwareSerial.h>
#include <string.h>
#include <inttypes.h>
#include "logging.h"

#define EH_STD_SERIAL 0
#define EH_SOFT_SERIAL 1

#define EH_config_std            Logger.config_std()
#define EH_config_soft(soft)     Logger.config_soft(&soft)

void EH_test();
extern uint8_t derr;
extern uint8_t errno;
extern char *errmsg;
extern char *EH_CLEAR_ERROR_MSG;
extern char *EH_EMPTY_STR;

void clrerr();
void seterr(uint8_t error);

#define EH_DW(code) do{code}while(0) //wraps in a do while(0) so that the syntax is correct.

#define raise(E, ...)                       EH_DW(EH_ST_raisem(E, __VA_ARGS__); goto error;)
#define assert(A)                           EH_DW(if(!(A)) {seterr(ERR_ASSERT); log_err(); goto error;})
#define assert_raise(A, E, ...)             EH_DW(if(!(A)) {raise((E), __VA_ARGS__);})

// These functions make the error label unnecessary
#define raise_return(E, ...)                    EH_DW(seterr(E); log_err(); return __VA_ARGS__;)
#define raisem_return(E, M, ...)                EH_DW(EH_ST_raisem(E, M); return __VA_ARGS__;)
#define assert_return(A, ...)                   EH_DW(if(!(A)){seterr(ERR_ASSERT); log_err(); return __VA_ARGS__;})
#define assert_raise_return(A, E, ...)          EH_DW(if(!(A)){seterr(E); log_err(); return __VA_ARGS__;})
#define assert_raisem_return(A, E, M, ...)      EH_DW(if(!(A)){EH_ST_raisem(E, M); return __VA_ARGS__;})


//#define iferr_return        if(derr) return 
//#define iferr_log_return    if(derr) {log_err();}  iferr_return
#define iferr_return(...)        EH_DW(if(derr) return __VA_ARGS__;)
#define iferr_log_return(...)    EH_DW(if(derr) {log_err(); return __VA_ARGS__;})
#define iferr_catch()            EH_DW(if(derr) goto error;)
#define iferr_log_catch()        EH_DW(if(derr) {log_err(); goto error;})

// Only log at the proper level.
#if LOGLEVEL >= LOG_DEBUG
  void EH_start_debug(char *file, unsigned int line);
  #define debug(...) EH_DW(LOG_IFLL(LOG_DEBUG, EH_start_debug(__FILE__, __LINE__); Logger.println(__VA_ARGS__);))
#else
  #define debug(...) 
#endif

#if LOGLEVEL >= LOG_INFO
void EH_start_info(char *file, unsigned int line);
  #define log_info(...) EH_DW(LOG_IFLL(LOG_INFO, EH_start_info(__FILE__, __LINE__); Logger.println(__VA_ARGS__);))
#else
  #define log_info(...) 
#endif

#if LOGLEVEL >= LOG_ERROR
  void EH_log_err(char *file, unsigned int line);
  #define EH_ST_raisem(E, ...) seterr(E); EH_DW(LOG_IFLL(LOG_ERROR, EH_log_err(__FILE__, __LINE__); Logger.println(__VA_ARGS__);))
  #define log_err(...)              EH_DW(LOG_IFLL(LOG_ERROR, EH_log_err(__FILE__, __LINE__); Logger.println(__VA_ARGS__);))
  #define clrerr_log()              EH_DW(LOG_IFLL(LOG_ERROR, seterr(ERR_CLEARED); log_err(); clrerr();))
  void EH_printerrno();
  void EH_printinfo(char *file, unsigned int line);
#else
  #define EH_ST_raisem(E, ...) seterr(E)
  #define log_err(...)
  #define clrerr_log() clrerr()
#endif


// Using pt (protothreads) library with debug.
// You can use the below functions OR you can define your own 
// error handling.
// If you do error handling, make PT_ERROR_OCCURED your last line.
//  (returns PT_ERROR)

#ifdef DEBUG000
#define PT_RAISE(pt, E) derr = (E); if(derr != pt->error){ errno=ERR_ASSERT; log_err(); Logger.println();} return PT_ERROR


#define PT_ERROR_OCCURED return PT_ERROR

// sets the pt location so that when it fails, it starts there again
#define PT_ERROR_TRY(pt) PTX_time = 0; LC_SET(pt)

// if PT_ASSERT fails, returns. On next call continues at place last set (use PT_ERROR_TRY)

#define PT_NOERR(pt) if(derr) return PT_ERROR
#endif
#endif



