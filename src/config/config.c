#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "../utils/hashmap.h"
#include "../utils/utils.h"

/*
 *
 *
 *
 */
static int str_hash_fn(void *str) {
  uint32_t hash = 5381;
  char *p;

  for (p = str; p && *p; p++)
    hash = ((hash << 5) + hash) + *p;
  return (int) hash;
}
/*
 *
 *
 *
 */
ConfigTable* config_table_new() {
  ConfigTable* cfg = malloc(sizeof(ConfigTable));
  cfg->mTable = hashmapCreate(36, str_hash_fn, hashmapIntEquals);
  cfg->mStatic = hashmapCreate(36, str_hash_fn, hashmapIntEquals);
  cfg->mOptional = hashmapCreate(36, str_hash_fn, hashmapIntEquals);
  return cfg;
}

/*
 *
 *
 *
 */
void config_table_free() {

  //TODO: очистим ресурсы памяти

}

/*Return true if the key is used in the table  */
bool isDefined(ConfigTable* cfg, char key[]) {
  return hashmapContainsKey(cfg->mTable, key);
}

/*Return true if this key is identified as static*/
bool isOptional(ConfigTable* cfg, char key[]) {
  return hashmapGet(cfg->mOptional, key); //values are 0 or 1;
}

/*Return true if this key is static, i.e can't be changed*/
bool isStatic(ConfigTable* cfg, char key[]) {
  return hashmapGet(cfg->mStatic, key); //values are 0 or 1
}

/*Make a key static*/
void makeStatic(ConfigTable* cfg, char key[]) {
  hashmapPut(cfg->mStatic, key, "1");
}

/*Make a key optional*/
void makeOptional(ConfigTable* cfg, char key[]) {
  hashmapPut(cfg->mStatic, key, "1");
}

/*Get string parameter from table */
char* getStr(ConfigTable* cfg, const char *key) {
  char *value;
  value = hashmapGet(cfg->mTable, (void *) key);

  strip_n(value);

  if (value)
    return value;
  else
    return "";
}

/*Get string parameter from table */
int* getArr(ConfigTable* cfg, const char *key) {
  char *value;
  value = hashmapGet(cfg->mTable, (void *) key);

  if (value)
    return value;
  else
    return 0;
}

/*Set or change value in the table */
void set(ConfigTable* cfg, char key[], char value[]) {
  hashmapPut(cfg->mTable, key, value);
}

/*Remove key from table */
bool unset(ConfigTable* cfg, char key[]) {
  return hashmapRemove(cfg->mTable, key);
}

/*Read table from config */
ConfigTable* readConfig(char *filename) {
  ConfigTable* cfg;
  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  cfg = config_table_new();

  fp = fopen(filename, "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

  while ((read = getline(&line, &len, fp)) != -1) {
    int i = 0;

    // skip string, which starts from space
    while (line[i] == ' ')
      continue;

    // skip comments line
    if (line[i] == '#')
      continue;

    // Skip blank lines
    if (line[i] == '\0')
      continue;

    int stringLength = 0;
    int x;
    int chnk = 0;
    /* Calculate number of tokens */
    stringLength = (int) strlen(line);
     for (x=0; x < stringLength; x++) {
       if(line[x]==' '){
         chnk++;
       }
     }
     chnk++;

    // Split string in series of tokens
    char *tokens[chnk]; // type name value
    int ret = splitString(line, tokens, ' ');

    // if string tokenized well
    if (ret > 1) {

      if (strcmp(tokens[3], "") == 0) {
        // Значит нужно считать как значение
        hashmapPut(cfg->mTable, tokens[1], tokens[2]);
      } else {
        // Значит нужно считать массив значений
        hashmapPut(cfg->mTable, tokens[1], getArray(tokens, chnk));
      }

      if (strcmp(tokens[0], "$static") == 0) {
        hashmapPut(cfg->mStatic, tokens[1], "1");
        hashmapPut(cfg->mOptional, tokens[1], "0");
      } else {
        hashmapPut(cfg->mOptional, tokens[1], "1");
        hashmapPut(cfg->mStatic, tokens[1], "0");
      }
    }
  }

  //if (line)
  //  free(line);

  return cfg;
}

/*Write table to config */
void writeConfig(char* filename) {

  FILE* fp;

  fp = fopen(filename, "w");



}

int getArray(char *tokens[], int length){

  int i, j, *values;
  values = malloc(sizeof(int)*length);
  for(i=2, j=0; i< length; i++, j++){
    values[j]= strtol(tokens[i], NULL, 10);
  }

  return values;
}

//static bool str_eq(void *key_a, void *key_b) {
//  return !strcmp((const char *) key_a, (const char *) key_b);
//}

