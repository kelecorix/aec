#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "config.h"
#include "../utils/hashmap.h"
#include "../utils/utils.h"
#include "../hw/site.h"

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

/* Функция создания новой таблицы конфигурации
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

/* Return true if the key is used in the table
 *
 */
bool isDefined(ConfigTable* cfg, char key[]) {
  return hashmapContainsKey(cfg->mTable, key);
}

/* Return true if this key is identified as static
 *
 */
bool isOptional(ConfigTable* cfg, char key[]) {
  return hashmapGet(cfg->mOptional, key); //values are 0 or 1;
}

/* Return true if this key is static, i.e can't be changed
 *
 */
bool isStatic(ConfigTable* cfg, char key[]) {
  return hashmapGet(cfg->mStatic, key); //values are 0 or 1
}

/* Make a key static
 *
 *
 */
void makeStatic(ConfigTable* cfg, char key[]) {
  hashmapPut(cfg->mStatic, key, "1");
}

/* Make a key optional
 *
 *
 */
void makeOptional(ConfigTable* cfg, char key[]) {
  hashmapPut(cfg->mStatic, key, "1");
}

/* Get string parameter from table
 *
 */
char* getStr(ConfigTable* cfg, const char *key) {
  char *value;
  value = hashmapGet(cfg->mTable, (void *) key);

  strip_n(value);

  if (value)
    return value;
  else
    return "";
}

/* Get array value from table
 *
 */
int* getArrI(ConfigTable* cfg, const char *key) {

  //int *mnm= NULL, *values=NULL;
  int *lvalues = (int *) hashmapGet(cfg->mTable, (void *) key);

//  int length = lvalues[0];// первый элемент это размер массива
//
//  mnm = (int *) realloc (values, length * sizeof(int));
//
//  if (mnm!=NULL) {
//    values=mnm;
//  }
//  else {
//    free (values);
//    puts ("Error (re)allocating memory");
//  }
//
//  memmove(values, (lvalues+1), (length-1));
  return (lvalues+1);

}

float* getArrF(ConfigTable* cfg, const char *key) {

  float *lvalues = (float *) hashmapGet(cfg->mTable, (void *) key);
  return (lvalues+1);

}


/* Compare 2 arrays
 *
 */
int cmpArr(int* values1, int* values2){

  int i, size1, size2, eq=1; // 0 - array are not equal 
  // check sizes
  size1 = sizeof(values1) / sizeof(int);
  size2 = sizeof(values2) / sizeof(int);

  if (size1 != size2) 
    return eq=0;

  // sizes are ok compare each element
  for (i=0; i < size1; i++) {
    if (values1[i] != values2[i]){
      eq = 0;
      break;
    }
  }
    
  return eq;

}


/* Set or change value in the table
 *
 */
void set(ConfigTable* cfg, char key[], char value[]) {
  hashmapPut(cfg->mTable, key, value);
}

/* Remove key from table
 *
 */
bool unset(ConfigTable* cfg, char key[]) {
  return hashmapRemove(cfg->mTable, key);
}

/* Read table from config
 *
 */
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

      if (chnk <= 3) {
        // Значит нужно считать как значение
        hashmapPut(cfg->mTable, tokens[1], tokens[2]);
      } else {
        // Значит нужно считать массив значений
        int ret, *values; // первые 2 значения имена, 3й тип массива еще 1 слот под значение длины

        if (strcmp(tokens[2], "int") == 0 ){
          values = malloc((chnk-2)*sizeof(int));
          ret = getArrayI(values, tokens, chnk);
        }

        if (strcmp(tokens[2], "float") == 0){
          values = malloc((chnk-2)*sizeof(float));
          ret = getArrayF(values, tokens, chnk);
        }

        hashmapPut(cfg->mTable, tokens[1], values);
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

/* Write table to config
 *
 *
 */
void writeConfig(char* filename) {

  FILE *fp, *fp2;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  char* key = NULL;
  char* value = NULL;

  //ConfigTable* cfg = config_table_new();

  fp = fopen(filename, "r");
  fp2 = fopen("replica", "w");

  if (fp == NULL)
    exit(EXIT_FAILURE);

  while ((read = getline(&line, &len, fp)) != -1) {
    int i = 0;

    // copy string, which starts from space
    if (line[i] == ' ')
      fwrite(line, sizeof(char), sizeof(line), fp2);

    // copy comments line
    if (line[i] == '#')
      fwrite(line, sizeof(char), sizeof(line), fp2);

    // copy blank lines
    if (line[i] == '\0')
      fwrite(line, sizeof(char), sizeof(line), fp2);

    int stringLength = 0;
    int x;
    int chnk = 0;

    /* Calculate number of tokens */
    stringLength = (int) strlen(line);
    for (x = 0; x < stringLength; x++) {
      if (line[x] == ' ') {
        chnk++;
      }
    }
    chnk++;

    // Split string in series of tokens
    char *tokens[chnk]; // type name value
    int ret = splitString(line, tokens, ' ');

    // if string tokenized well
    if (ret > 1) {
      if (chnk < 3) {
        // Значит нужно считать как значение
        key = tokens[1];
        value = tokens[2];

        // compare current value, with value from hashtable
        if (value != hashmapGet(site->cfg, key)) {
          //value changed
          //write new value
          //fwrite();
        } else {
          // write as is
          fwrite(value, sizeof(char), sizeof(value), fp2);
        }

      } else {
        // Значит нужно считать и записать массив значений
        if (strcmp(tokens[2], "int")) {
          int length = 0, value, *values;
          char *val="";
          key = tokens[1];
          values = getArrayI(values, tokens, chnk);

          // compare array of values, with values from hashtable
          if (memcmp(values, hashmapGet(site->cfg, key), length)==1) {
            // some values changed, so rewrite hole array
            // in it's current condition
            // write new value
            for (i = 0; i < length; i++) {
              fwrite(itoa(values[i], val, 10), sizeof(char), sizeof(value), fp2);
            }
          }

          if (strcmp(tokens[2], "float")) {
            int length=0;
            float value, *values;
            key = tokens[1];
            values = getArrayF(values, tokens, chnk);

          }
        } else {
          // write as is
          // fwrite();
        }
      }

    }
  }
}

/* Первое значение values длина массива
 *
 *
 */
int getArrayI(int *values, char *tokens[], int length){

  int i, j;
  //values = malloc(sizeof(int)*length);
  values[0] = length-3;// первые 2 элемента это опция и имя, 3й тип
  for(i=3, j=1; i< length; i++, j++){
    values[j]= strtol(tokens[i], NULL, 10);
  }

  return 0;
}

int getArrayF(float *values, char *tokens[], int length){

  int i, j;
  //values = malloc(sizeof(int)*length);
  values[0] = length-3;// первые 2 элемента это опция и имя, 3й тип
  for(i=3, j=1; i< length; i++, j++){
    values[j]= strtof(tokens[i], NULL);
  }

  return 0;
}

void test_config(){

  // 2. Беспорядочно меняем значения
  // 3. Беспорядочно меняем значения в массиве
  // 4. Выполняем запись нового конфига

  hashmapPut(site->cfg->mTable, "temp_support", rand());
  hashmapPut(site->cfg->mTable, "temp_dew", rand());

  int i, *arr;
  arr = malloc(10*sizeof(int));
  for(i=0; i<10; i++){
    arr[i] = rand();
  }

  hashmapPut(site->cfg->mTable, "vent2_steps", arr);

  writeConfig(concat(gcfg->cdir, "freecooling.conf"));

}

//static bool str_eq(void *key_a, void *key_b) {
//  return !strcmp((const char *) key_a, (const char *) key_b);
//}

