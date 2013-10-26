#ifndef SITE_C_
#define SITE_C_

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "throttle.h"
#include "vent.h"
#include "site.h"
#include "ac.h"
#include "ow.h"
#include "i2c.h"
#include "../config/config.h"
#include "../log/logger.h"

/*
 *
 *
 *
 */
typedef struct Cfg {
  int debug; // Debug Level: 1,2,3,4
  int gpf;   // Global print flag
  int motodrop;
  char* saddr; // адрес owfs сервера
  char* mpoint; // адрес для считывания owfs
  char* ldir; // директория для записи логов
  char* cdir; // директория конфигурационного файла
  int ltime;  // периодичность записи глобального лога
  int mtime;  // периодичность записи моточасов
  char* filename; // имя файла конфигурации
  char* edir;  //директория для файло эмуляции
  Logger* logger;
  OWNET_HANDLE conn;
} Cfg;

/*
 *
 *
 *
 */
typedef struct Site {

  AC** acs; // массив кондционеров в соотв. с конфигурацией, обычно 1 или 2
  Vent* vents[2]; // массив вентиляторов, обычно 2 один приточный другой вытяжной
  Throttle* th;
  int ten; // Управление теном 0 - выкл.вок
  int num_ac; // количество кондиционеров

  int mode; // Текущий режим работы сайта 0, 1, 2, 3, 4, 5, 6
  // Режим УВО, Режим охлаждения кондиционером, Режим догрева сайта, Аварийный режим смежный, Аварийный режим УВО, Аварийный режим кондиционера

  int power; // 0 - OFF/питания нет, 1 - ON/питание есть

  float temp_in;      // температура сайта
  float temp_in_prev; // предыдущая температура сайта
  float temp_in_prev_prev; //пред. предыдущая температура сайта
  float temp_out;     // температура окружающей среды
  float temp_mix;     // температура камеры смешения
  float temp_evapor1; // температуры испарителя 1го кондиционера
  float temp_evapor2; // температуры испарителя 2го кондиционера
  int tacho1;         // шаг таходатчика 1
  int tacho2;         // шаг таходатчика 2
  int tacho1_t;       // обороты таходатчика 1
  int tacho2_t;       // обороты таходатчика 2
  int th_r;           // положение заслонки считанное

  int th_exists;      // cуществует заслонка в виде железа или нет
  int th_r_exists;       // существует адрес считывания положения заслонки
  int tacho1_exists;   // таходатчик1 существует
  int tacho2_exists;   // таходатчик2 существует

  int penalty; // штраф
  int th_check; // throttle check - флаг проверки заслонки
  time_t time_pre; // время предыдущего решения в секундах
  time_t time_uvo; // время включения охлаждения с помощью вентиляторов
  ConfigTable* cfg; // configuration for this particular  site

  int (*set_mode)(struct Site*, int value);
  int (*set_ten)(struct Site*, int value);
  double (*get_ac_time_work)(struct Site*);

} Site;



/*
 *
 *
 *
 */
extern Site* site;

extern Cfg* gcfg;

// Режимы работы 
int site_mode_uvo();
int sub_uvo_pen();
void sub_uvo_vent();
void sub_uvo_pow();
void sub_uvo_th(int fail);
int sub_uvo_fail();
int site_mode_ac();
int site_mode_heat();
int site_mode_fail_uvo(char* from);
int site_mode_fail_ac();
int site_mode_fail_mix();
int read_sensors();
ConfigTable* read_config(char* filename);
void run();
void run_moto();
void pop_hw();
int search_sensors(OWNET_HANDLE conn);
Site* site_new();
Cfg* new_gcfg();
#endif /*SITE_C_*/
