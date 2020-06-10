#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "runtime.h"

__attribute__((noreturn))
static void error(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

void __print_err(const char *s) {
  if(!fprintf(stderr, "%s", s)) {
    exit(EXIT_FAILURE);
  }
}

void __print(const char *s) {
  if(!fprintf(stdout, "%s", s)){
    exit(EXIT_FAILURE);
  }
}

void __print_int(const int32_t i) {
  if(!fprintf(stdout, "%d", i)){
    exit(EXIT_FAILURE);
  }
}

void __flush(void) {
  fflush(stdout);
}

const char *__getchar(void) {
  char str[2];
  // Get the first character with the terminating null character
  if(fgets(str, 2, stdin)) {
    char * myStr = (char *) malloc(sizeof(char));
    myStr[0] = '\0';
    return myStr;
  }
  else {
    char * myStr = (char *) malloc(2*sizeof(char));
    *myStr = *str;
    return myStr;
  }
}

int32_t __ord(const char *s) {
  if(!s) {
    return -1;
  }
  else {
    return (int32_t)s[0];
  }
}

const char *__chr(int32_t i) {
  error("UNIMPLEMENTED __chr");
}

int32_t __size(const char *s) {
  error("UNIMPLEMENTED __size");
}

const char *__substring(const char *s, int32_t first, int32_t length) {
  error("UNIMPLEMENTED __substring");
}

const char *__concat(const char *s1, const char *s2) {
  error("UNIMPLEMENTED __concat");
}

int32_t __strcmp(const char *s1, const char *s2) {
  error("UNIMPLEMENTED __strcmp");
}

int32_t __streq(const char *s1, const char *s2) {
  error("UNIMPLEMENTED __streq");
}

int32_t __not(int32_t i) {
  error("UNIMPLEMENTED __not");
}

void __exit(int32_t c) {
  error("UNIMPLEMENTED __exit");
}
