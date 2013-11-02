#ifndef UTILS_C_
#define UTILS_C_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int splitString(char *string, char *items[], char splitChar);
int splitStringA(char *string, char *items, char splitChar);
char* concat(char* str1, char* str2);
int concat3(char *first, char *second, char *third);
char* string_copy(const char* s);
char* trim(char *s);
char* ltrim(char *s);
char* rtrim(char *s);
void strip_n(char* s);
void strip_t(char* s);
void strip_log(char* s);
void revS(char str[]);
void reverse(char s[]);
void itoa(int n, char s[]);

#endif /*UTILS_C_*/
