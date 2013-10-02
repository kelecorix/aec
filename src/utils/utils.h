#ifndef UTILS_C_
#define UTILS_C_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int splitString(char *string, char *items[], char splitChar);
char* concat(char* str1, char* str2);
int concat3(char *first, char *second, char *third);
char* string_copy(const char* s);
char* trim(char *s);
char* ltrim(char *s);
char* rtrim(char *s);
void strip_n(char* s);
void revS(char str[]);
char* get_app_dir();

#endif /*UTILS_C_*/
