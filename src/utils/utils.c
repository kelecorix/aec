#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "utils.h"

/*
 * This splits a string into X parts given the split character.
 */
int splitString(char *string, char *items[], char splitChar) {
  /* Declare local variables.  */
  char temp[128];
  int stringLength = 0;
  int chunks = 0;
  int pos = 0;
  int x;

  /* Check if the given string is NULL.  */
  if (string == (char *) NULL) {
    return (0);
  }

  /* Get the length of the string.  */
  stringLength = (int) strlen(string);
  if (stringLength == 0) {
    return (0);
  }

  /* Zero out the temp string. */
  //memset (temp, 0, sizeof(temp));
  /* Start looking for the string. */
  for (x = 0; x < stringLength; x++) {
    if (string[x] == splitChar) {
      /* Copy the string into the array. */
      temp[pos++] = 0;
      items[chunks++] = strdup(temp);

      /* Zero out the temp string.  */
      memset(temp, sizeof(temp), '\0');

      /* Reset the position counter.  */
      pos = 0;
    } else {
      temp[pos++] = string[x];
    }
  }

  /* Don't forget the last one.  */
  temp[pos++] = 0;
  items[chunks++] = strdup(temp);
  return chunks;
}

/* Smart strings concatenation*/
char* concat(char* str1, char* str2) {

  /* Create a new string with a needed length */
  int size = strlen(str1) + strlen(str2);
  char* new = calloc((size + 1), sizeof(char));

  /* Concat the strings */
  strcat(new, str1);
  strcat(new, str2);
  return new;
}

/*
 *
 *
 *
 */
int concat3(char *first, char *second, char *third) {

  //char *total;

  return 0;

}

/*
 *
 *
 *
 */
char* string_copy(const char* s) {
  char* result;
  int l;

  if (s == 0)
    return 0;

  l = strlen(s) + 1;

  result = (char*) malloc(sizeof(char) * l);
  memcpy(result, s, l);

  return result;
}

/*
 *
 *
 *
 */
void strip_n(char* s) {
  char *pos;
  if ((pos = strchr(s, '\n')) != NULL)
    *pos = '\0';
}

/*
 *
 *
 *
 */
void strip_breaks(char* str) {
  char *write = str, *read = str;
  do
    while ((*read) == '\n')
      read++;
  while ((*write++ = *read++));
}

/*
 *
 *
 *
 */
void strip_tabs(char* str) {
  char *write = str, *read = str;
  do
    while ((*read) == '\t')
      read++;
  while ((*write++ = *read++));
}

/*
 *
 *
 *
 */
char* ltrim(char *s) {
  while (isspace(*s))
    s++;
  return s;
}

/*
 *
 *
 *
 */
char* rtrim(char *s) {
  char* back = s + strlen(s);
  while (isspace(*--back))
    ;
  *(back + 1) = '\0';
  return s;
}

/*
 *
 *
 *
 */
char* trim(char *s) {
  return rtrim(ltrim(s));
}

/*
 *
 *
 *
 */
void revS(char str[])
{
  char c;
  char *p, *q;

  p = str;
  if (!p)
    return;

  q = p + 1;
  if (*q == '\0')
    return;

  c = *p;
  revS(q);

  while (*q != '\0') {
    *p = *q;
    p++;
    q++;
  }
  *p = c;

  return;
}

