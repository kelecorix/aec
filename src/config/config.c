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

  int *lvalues = (int *) hashmapGet(cfg->mTable, (void *) key);
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

      if (chnk <= 4) {
        // Значит нужно считать как значение
        strip_n(tokens[3]);
        hashmapPut(cfg->mTable, tokens[1], tokens[3]);

      } else {
        // Значит нужно считать массив значений
        // первые 2 значения имена, 3й тип массива еще 1 слот под значение длины

        if (strcmp(tokens[2], "int") == 0 ){
          int *values;
          values = malloc((chnk-2)*sizeof(int));
          getArrayI(values, tokens, chnk);
          hashmapPut(cfg->mTable, tokens[1], values);
        }
        if (strcmp(tokens[2], "float") == 0){
          float *values;
          values = malloc((chnk-2)*sizeof(float));
          getArrayF(values, tokens, chnk);
          hashmapPut(cfg->mTable, tokens[1], values);
        }
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
  char* type = NULL;

  fp = fopen(filename, "r");
  fp2 = fopen("replica", "w");

  if (fp == NULL)
    exit(EXIT_FAILURE);

  while ((read = getline(&line, &len, fp)) != -1) {
    int i = 0;

    // copy string, which starts from space
    if (line[i] == ' ') {
      fprintf(fp2, line);
      fprintf(fp2, "\n");
      continue;
    }

    // copy comments line
    if (line[i] == '#') {
      fprintf(fp2, line);
      continue;
    }

    // copy blank lines
    if (line[i] == '\0') {
      fprintf(fp2, line);
      fprintf(fp2, "\n");
      fprintf(fp2, "\n");
      continue;
    }

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
      if (chnk <= 4) {
        // Значит нужно считать как значение
        key = tokens[1];
        type = tokens[2];
        char *cvalue = tokens[2];
        char *tvalue = hashmapGet(site->cfg->mTable, key);
        // compare current value, with value from hashtable
        if (strcmp(cvalue, tvalue) != 0) {
          //value changed
          //write new value
          fprintf(fp2, tokens[0]);
          fprintf(fp2, " ");
          fprintf(fp2, tokens[1]);
          fprintf(fp2, " ");
          fprintf(fp2, tokens[2]);
          fprintf(fp2, " ");

          if ((strcmp(type, "int") == 0) || (strcmp(type, "string") == 0))
            fprintf(fp2, tvalue);

          if (strcmp(type, "hex") == 0) {
            int val = strtol(tvalue, NULL, 16);
            fprintf(fp2, "0x%x", val);
          }

          fprintf(fp2, "\n");
          fprintf(fp2, "\n");

        } else {
          // write as is
          fprintf(fp2, line);
          fprintf(fp2, "\n");
          fprintf(fp2, "\n");
        }

      } else {
        // Значит нужно считать и записать массив значений
        if (strcmp(tokens[2], "int") == 0) {
          int length = 0, value, *values = NULL, *cvalues, *values2;

          key = tokens[1];

          int j;
          cvalues = malloc((chnk - 3) * sizeof(int));
          for (j = 3, i = 0; i < chnk - 3; i++, j++) {
            cvalues[i] = strtol(tokens[j], NULL, 10); // values read from config
          }

          values = hashmapGet(site->cfg->mTable, key); // 1й элемент длина массива
          values2 = malloc(values[0] * sizeof(int));
          memcpy(values2, values + 1, values[0] * sizeof(int));
          int bt = sizeof(cvalues);

          // compare array of values, with values from hashtable
          if (memcmp(values2, cvalues, bt) > 0) {
            // some values changed, so rewrite hole array
            // in it's current condition
            // write new value
            fprintf(fp2, tokens[0]);
            fprintf(fp2, " ");
            fprintf(fp2, tokens[1]);
            fprintf(fp2, " ");
            fprintf(fp2, tokens[2]);
            fprintf(fp2, " ");
            length = chnk - 3;
            for (i = 0; i < length; i++) {
              fprintf(fp2, "%d", values2[i]);
              fprintf(fp2, " ");
            }
            fprintf(fp2, "\n");
            fprintf(fp2, "\n");
          } else {
            fprintf(fp2, line);
            fprintf(fp2, "\n");
          }
        }

        if (strcmp(tokens[2], "float") == 0) {
          int length = 0, value;
          float *values = NULL, *cvalues, *values2;

          key = tokens[1];

          int j;
          cvalues = malloc((chnk - 3) * sizeof(float));
          for (j = 3, i = 0; i < chnk - 3; i++, j++) {
            cvalues[i] = strtol(tokens[j], NULL, 10); // values read from config
          }

          values = hashmapGet(site->cfg->mTable, key); // 1й элемент длина массива
          values2 = malloc(values[0] * sizeof(float));
          memcpy(values2, values + 1, values[0] * sizeof(float));
          int bt = sizeof(cvalues);

          // compare array of values, with values from hashtable
          if (memcmp(values2, cvalues, bt) > 0) {
            // some values changed, so rewrite hole array
            // in it's current condition
            // write new value
            fprintf(fp2, tokens[0]);
            fprintf(fp2, " ");
            fprintf(fp2, tokens[1]);
            fprintf(fp2, " ");
            fprintf(fp2, tokens[2]);
            fprintf(fp2, " ");
            length = chnk - 3;
            for (i = 0; i < length; i++) {
              fprintf(fp2, "%f", values2[i]);
              fprintf(fp2, " ");
            }
            fprintf(fp2, "\n");
            fprintf(fp2, "\n");
          } else {
            fprintf(fp2, line);
            fprintf(fp2, "\n");
          }
        }
      }
    }
  }
  fclose(fp);
  fclose(fp2);

  //replace config with replica
  remove(fp);
  rename("replica", filename);

}

/* Первое значение values длина массива
 *
 *
 */
void getArrayI(int *values, char *tokens[], int length){
  int i, j;
  values[0] = length-3;// первые 2 элемента это опция и имя, 3й тип
  for(i=3, j=1; i< length; i++, j++){
    values[j]= strtol(tokens[i], NULL, 10);
  }
}

void getArrayF(float *values, char *tokens[], int length){
  int i, j;
  values = malloc(sizeof(int)*length);
  values[0] = length-3;// первые 2 элемента это опция и имя, 3й тип
  for(i=3, j=1; i< length; i++, j++){
    values[j]= strtof(tokens[i], NULL);
  }
}

void test_config(){

  int val;
  char cval[20];
  val = rand() % 20;
  sprintf(cval, "%d", val);
  hashmapPut(site->cfg->mTable, "temp_support", cval);
  val = rand() % 20;
  sprintf(cval, "%d", val);
  hashmapPut(site->cfg->mTable, "temp_dew", cval);

  int i, *arr;
  arr = malloc(10*sizeof(int));
  for(i=0; i<10; i++){
    arr[i] = rand() % 1000;
  }

  hashmapPut(site->cfg->mTable, "vent2_steps", arr);

  //if(temp_support < -20.f) {
    writeConfig(concat(gcfg->cdir, "freecooling.conf"));
  //}

}

//static bool str_eq(void *key_a, void *key_b) {
//  return !strcmp((const char *) key_a, (const char *) key_b);
//}

