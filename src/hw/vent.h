#ifndef VENT_H_
#define VENT_H_

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

typedef enum {
  NOERROR, ERROR, ERROR_HEAT
} vent_error;

//Ventilation
typedef struct Vent {

  int mode; // 0 - OFF/ВЫКЛ, 1 - ON/ВКЛ
  int turns; // обороты, значения мз таблицы 
  long time_work;
  time_t time_start;
  time_t time_stop;

  vent_error error;

  int (*set_mode)(struct Vent*, int val);
  int (*set_turns)(struct Vent*, int val);

} Vent;

#endif /*VENT_H_*/
