#ifndef VENT_H_
#define VENT_H_

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "i2c.h"

typedef enum {
  NOERROR, ERROR, ERROR_HEAT
} vent_error;

//Ventilation
typedef struct Vent {

  int mode; // 0 - OFF/ВЫКЛ, 1 - ON/ВКЛ
  int turns; // обороты, значения мз таблицы 
  int type; // 0 - приточный/in, 1 - вытяжной
  long time_work;
  time_t time_start;
  time_t time_stop;

  vent_error error;

  int steps[10] = { 0xFF, 0xED, 0xDF, 0xDE, 0xDC, 0xBF, 0xBE, 0x7F, 0x7E, 0x9F,
       0x8F};

  int (*set_mode)(struct Vent*, int val);
  int (*set_turns)(struct Vent*, int val);

} Vent;

#endif /*VENT_H_*/
