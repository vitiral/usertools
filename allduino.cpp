
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

