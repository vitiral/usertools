
/**
 * Better strncpy
 * Example code:
 * char buf[100];
 * uint16_t len = 100;
 * char *place = buf;
 * len = bstrncpy(&place, "hello\n", len);
 * len = bstrncpy(&place, "another hi\n", len);
 * input:
 *   char **destination -- pointer to the buffer where you want the text.
 *     The location WILL be altered to the end of the character array.
 *  char *source -- character array to copy to destination
 *  len -- length of the array (remaining)
 *
 * This function ensures that the last character is a 0 -- no matter what.
 *
 * returns the remaining length of the array (based on the input len)
 **/

#include "allduino.h"

uint16_t bstrncpy(char **destination, const char *source, uint16_t len){
    uint16_t n;
    for(n = 0; (n < len - 1) and (source[n] != 0); n++){
        (*destination)[n] = source[n];
    }
    (*destination)[n] = 0;
    (*destination) += n;
    return len - n;
}

char *d2strf (double val, signed char width, unsigned char prec, char *sout) {
  char fmt[20];
  sprintf(fmt, "%%%d.%df", width, prec);
  sprintf(sout, fmt, val);
  return sout;
}
