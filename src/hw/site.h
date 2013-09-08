#ifndef SITE_C_
#define SITE_C_

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "site.h"
#include "ac.h"
#include "throttle.h"
#include "vent.h"
#include "ow.h"
#include "../config/config.h"

typedef struct Site {

  // Hardware
  // Оборудование
  AC* acs[2]; // массив кондмцмонеров в соотв. с конфигурацией, обычно 1 или 2
  Vent* vents[2]; // массив вентиляторов, обычно 2 один приточный другой вытяжной
  Throttle* th;
  int ten; // Управление теном 0 - выкл.вок

  int mode; // Текущий режим работы сайта 0, 1, 2, 3, 4, 5, 6
  // Режим УВО, Режим охлаждения кондиционером, Режим догрева сайта, Аварийный режим смежный, Аварийный режим УВО, Аварийный режим кондиционера

  int power; // 0 - OFF/питания нет, 1 - ON/питание есть

  float temp_in;      // температура сайта
  float temp_in_prev; // предыдущая температура сайта
  float temp_out;     // температура окружающей среды
  float temp_mix;     // температура камеры смешения
  float temp_evapor1; // температуры испарителя 1го кондиционера
  float temp_evapor2; // температуры испарителя 2го кондиционера

  int penalty; // штраф
  int th_check; // throttle check - флаг проверки заслонки
  time_t time_pre; // время предыдущего решения в секундах
  time_t time_uvo; // время включения охлаждения с помощью вентиляторов
  ConfigTable* cfg; // configuration for this particular  site

  OWNET_HANDLE conn;
  char* mount_point;

  int (*set_mode)(struct Site*, int value);
  int (*set_ten)(struct Site*, int value);
  double (*ac_time_work)(struct Site*);

} Site;

// Режимы работы 
int site_mode_uvo(Site* site);
int site_mode_ac(Site* site);
int site_mode_heat(Site* site);
int site_mode_fail_uvo(Site* site);
int site_mode_fail_ac(Site* site);
int site_mode_fail_mix(Site* site);
int read_sensors();
ConfigTable* read_config(char* filename);
void run(Site* site);

#endif /*SITE_C_*/
