#define DEBUG

#include <strtools.h>
#include <errorhandling.h>
#include <ByteBuffer.h>

ByteBuffer failbuffer;
uint8_t testerr = 0;
#define test_assert(A) EH_DW(if(!(A)){seterr(ERR_CRITICAL); testerr = ERR_CRITICAL; log_err(); clrerr();})

void main_test(uint8_t ti){
  char txt[100];
  char *t2 = txt;
  char *word;
  uint8_t i;
  
  switch(ti){
    case 0:
      debug("flash");
      flash_to_str(F("Hello"), txt);
      test_assert(strcmp("Hello", txt) == 0);
      test_assert(cmp_str_flash(txt, F("Hello")));
      break;
    case 1:
      debug("number conversions");
      test_assert(get_int("42") == 42);
      test_assert(get_int("-56") == -56);
      
      //test_assert(get_float("0f800.23") == 800.23);
      //test_assert(get_float("-0f400.23" == -400.23));
      break;
      
    case 2:
      debug("strip");
      flash_to_str(F("  \n I was stripped \n\r  \t"), txt);
      t2 = txt;
      debug((uint8_t)t2[strlen(t2)], HEX);
      t2 = strip(t2, strlen(t2));
      debug(t2);
      test_assert(strcmp("I was stripped", t2) == 0);
      break;
    case 3:
      debug("getting words");
      for( i = 0; i < 2; i++){
        flash_to_str(F("  Hi   23   We \n  are   Bob   \n"), txt);
        t2 = txt;
        t2 = strip(t2, strlen(t2));
        word = get_word(t2);
        word = strip(word, strlen(word));
        test_assert(strcmp("Hi", word) == 0);
        
        word = get_word(t2);
        word = strip(word, strlen(word));
        test_assert(strcmp("23", word) == 0);
        
        word = get_word(t2);
        word = strip(word, strlen(word));
        test_assert(strcmp("We", word) == 0);
        
        word = get_word(t2);
        word = strip(word, strlen(word));
        test_assert(strcmp("are", word) == 0);
        
        word = get_word(t2);
        word = strip(word, strlen(word));
        test_assert(strcmp("Bob", word) == 0);
      }
      break;
  }
}

void setup(){
  failbuffer.init(100);
  Serial.begin(57600);
  Serial.println(F("*** SETUP BEGINS ***"));
  log_info(F("all setup"));
}

#define T1_TESTS 5
void loop(){
  int out;
  int failures = 0;

  Logger.println(F("\n\n\n\n"));
  Logger.println(F("Testing Standard:"));
  for(int n = 0; n <= T1_TESTS; n++){
    Logger.flush();
    Logger.print(F("Testcase:"));
    Logger.println(n);

    clrerr();
    testerr = 0;
    main_test(n);
    if(testerr){
      Logger.println(F("*** FAILURE ***"));
      log_err();
      failures++;
      failbuffer.put(n);
      while(1);
    }
    else{
      Logger.println(F("*** Success ***"));
    }
  }

  Logger.println(F("##### Tests Done #####"));
  Logger.print(F("Results:  ")); 
  Logger.print(failures); 
  Logger.print(F(" failures out of: ")); 
  Logger.println(T1_TESTS);
  if(failbuffer.getSize()) {
    Logger.println(F("Failed cases:"));
  }

  while(failbuffer.getSize() > 0){
    Logger.print((unsigned short)failbuffer.get());
    Logger.print(F(", "  ));
  }

  L_println("\n!!!! DONE!");
  while(true);
}

