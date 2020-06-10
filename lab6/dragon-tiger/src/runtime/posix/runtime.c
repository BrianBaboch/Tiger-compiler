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
  if(fgets(str, 2, stdin) == NULL) {
    char * myStr = malloc(sizeof(char));
    myStr[0] = '\0';
    return myStr;
  }
  else {
    char * myStr = malloc(2*sizeof(char));
    myStr[0] = str[0];
    myStr[1] = '\0';
    return myStr;
  }
}

int32_t __ord(const char *s) {
  if(!s[0]) {
    return -1;
  }
  else {
    return (unsigned char)s[0];
  }
}

const char *__chr(int32_t i) {
  char * myStr = malloc(2*sizeof(char));
  if(i == 0) {
    myStr[0] = '\0';	  
  }
  else if(i < 0 || i > 255) {
    exit(EXIT_FAILURE);
  }
  else {
    myStr[0] = (char)i;
  }
  return myStr;
}

int32_t __size(const char *s) {
  return strlen(s);
}

const char *__substring(const char *s, int32_t first, int32_t length) {
  if(first < 0 || length <0) {
    exit(EXIT_FAILURE);
  }
  if(strlen(s) < length + first) {
    exit(EXIT_FAILURE);
  }
 
  char * myStr = malloc((length+1)*sizeof(char));
  for(int i = 0; i < length; i++) {
    myStr[i] = s[first + i];
  }

  myStr[length] = '\0';
  return myStr;
}

const char *__concat(const char *s1, const char *s2) {
  char * myStr = malloc((strlen(s1) + strlen(s2))*sizeof(char));
  for(int i = 0; i < strlen(s1); i++) {
    myStr[i] = s1[i];
  }
  for(int i = 0; i < strlen(s2); i++) {
    myStr[strlen(s1) + i] = s2[i];
  }
  return myStr;
}

int32_t __strcmp(const char *s1, const char *s2) {
  if(!strcmp(s1, s2)) {
    return 0;
  }
  else {
    if(strcmp(s1, s2) < 0) {
      return -1;
    }
    else {
      return 1;
    }
  }
}

int32_t __streq(const char *s1, const char *s2) {
  if(!strcmp(s1, s2)) {
    return 1;
  }
  else {
    return 0;
  }
}

int32_t __not(int32_t i) {
  if(i) {
    return 0;
  }
  else {
    return 1;
  }
}

void __exit(int32_t c) {
  exit(c);
}
