#ifndef AC_H_
#define AC_H_

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

typedef enum {
  NO_ERROR, NOPOWER, NODATA
} ac_error;

typedef struct AC {

  int mode; // 0 - OFF/ВЫКЛ, 1 - ON/ВКЛ
  double temp; // температура 
  time_t time_start;
  time_t time_stop;
  long time_work;
  int is_diff; // флаг набранной дельты

  long moto;

  ac_error error;

  int (*ac_start)(struct AC*);
  int (*ac_stop)(struct AC*);
  double (*ac_time_work)(struct AC*);
  long (*ac_moto_work)(struct AC*);

} AC;

#endif /*AC_H_*/
