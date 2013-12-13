
#include <Arduino.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "LinkedList.h"

#define DEBUG
#include <SoftwareSerial.h>
#include "errorhandling.h"

#include "ui.h"
#include "threading.h"
#include "strtools.h"

#define MAX_STR_LEN 100
#define MAX_ARRAY_LEN 256

//char *LINE_FEED = "\x0D\x0A";
char UI_CMD_END_CHAR = 0x0A;
char UI_CMD_PEND_CHAR = 0x0D;  // may be right before the end.
char *UI_TABLE_SEP = " \t ";

// #####################################################
// ### Functions

char *pass_ws(char *c){
  while(*c == ' ') {
    c++;
  }
  return c;
}

char *get_cmd_end(char *c){
  uint8_t k = false;
  while(*c != UI_CMD_END_CHAR){
    c++;
    k = true;
  }
  if(k){ // if the prev char is 0x0A, make that the end.
    if(*(c-1) == UI_CMD_PEND_CHAR) c -= 1;
  }
  assert_return(c, 0);
  return c;
}

char *get_word_end(char *c){
  c = pass_ws(c);
  while(*c != ' ' and *c != 0 and *c != 0x0A and *c != 0x0D){
    c++;
  }
  return c;
}

//char *word = get_word(c); // get first word
//char *word2 = get_word(c); // get next word
char *_get_word(char **c){
  *c = pass_ws(*c);
  char *word = *c;
  char *ce = get_word_end(*c);
  debug(ce - 1);
  *ce = 0;
  *c = ce + 1; // sets c to next word
  if(*word == 0) seterr(ERR_INPUT);
  return word;
}

long int _get_int(char **c){
  char *word = _get_word(c);
  iferr_log_return(0);
  return strtol(*c, NULL, 0);
}

void ui_process_command(char *c){
  char *c2;
  char *word;
  debug("Parse CMD");
  debug(*c);
  c = pass_ws(c);
  debug(*c);

  if(*c == '?'){
    c++;
    switch(*c){
    case 'p':
      //return value at pin #
      break;
    case 'v':
      debug("cmd 'v'");
      word = get_word(c); // skip first word, it is just 'v'
      iferr_log_catch();
      word = get_word(c);
      debug(String("var:") + word);
      print_variable(word);
      break;
    case 'f':
      // print out all exposed functions
      break;
    case 't':
      // print out running threads
      break;
    }   
  }

error:
  return;
}






