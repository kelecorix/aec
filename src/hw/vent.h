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

  int (*set_mode)(struct Vent*, int val);
  int (*set_turns)(struct Vent*, int val);

} Vent;

int i2c_get_tacho_data(struct Vent* v, int addr);
int turns_to_step(int turns, int type);

#endif /*VENT_H_*/
