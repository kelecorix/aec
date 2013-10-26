#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "../utils/hashmap.h"

typedef struct ConfigTable {

  Hashmap* mTable; /* The configuration table*/
  Hashmap* mStatic; /* Flags to indicate static config values*/
  Hashmap* mOptional; /* Flags to indicate optional config values*/

  bool (*isDefined)(struct ConfigTable*, char key[]);
  bool (*isStatic)(char key[]);
  bool (*isRequired)(char key[]);
  void (*set)(char key[], char val[]);
  bool (*unset)(char key[]);

  void (*makeStatic)(char* key[]);
  void (*makeOptional)(char* key);

  const char (*getNum)(char* key[]);

} ConfigTable;

char* getStr(ConfigTable* cfg, const char *key);
int getArrI(ConfigTable* cfg, const char *key, int *values);
int getArrF(ConfigTable* cfg, const char *key, float *values);
int getArray(int *values, char *tokens[], int length);
ConfigTable* readConfig(char* filename);
void writeConfig(char* filename);

/* Structure of configuration file 

 [Parameters]
 diff             - разница температура улица/помещ, разреш. охлажд. вентилят
 temp_support     - температура поддерживаемая в помещении
 temp_dew         - температура росы
 temp_fail        - аварийная температура в помещении
 temp_heat        - температура, когда необходимо вкл. обогрев
 num_ac           - количество кондиционеров
 time_ac1         - время наработки первого кондиционера
 time_ac2         - время наработки второго кондиционера
 time_flow_in     - время наработки приточного вентилятора
 time_flow_out    - время наработки вытяжного вентилятора

 [Sensors]
 s_temp_outdoor   - адрес датчика температуры улицы
 s_temp_indoor    - адрес датчика температуры помещения
 s_temp_mix       - адрес датчика температуры в камере смешения
 s_temp_evapor1   - адрес датчика температуры испарителя 1го кондиционера
 s_temp_evapor2   - адрес датчика температуры испарителя 2го кондиционера

 [Bus]
 a_1wire          - адрес микросхемы 1-wire
 a_lcd            - адрес микросхемы LCD (только запись)
 a_keyb           - адрес микросхемы клавиатура (чтение/запись)
 a_flow_in        - адрес микросхемы ЦАП приточного вентилятора
 a_flow_out       - адрес микросхемы ЦАП вытяжного вентилятора
 a_rtc            - адрес микросхемы часов реального времени
 a_tacho_flow_in  - адрес микросхемы тахо приточного вентилятора
 a_tacho_flow_out - адрес микросхемы тахо вытяжного вентилятора
 a_ac1_current    - адрес микросхемы АЦП ток потребления 1го кондиционера
 a_ac2_current    - адрес микросхемы АЦП ток потребления 2го кондиционера
 a_top_current    - адрес микросхемы АЦП ток потребления топовых огней
 a_voltage        - адрес микросхемы АЦП напряжения питания
 
 */

#endif /*CONFIG_H_*/
