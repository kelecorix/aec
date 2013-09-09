#ifndef THROTTLE_H_
#define THROTTLE_H_

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

// Заслонка
typedef struct Throttle {

  int mode; // 0 - сайт, 1 улица
  int position; // 0 -255
  int exist; // 1 - заслонка есть, 0 - заслонка отсутствует

  time_t time_start;
  time_t time_stop;
  long time_work;

  int (*set_mode)(struct Throttle*, int val);
  int (*set_position)(struct Throttle*, int val);

} Throttle;

#endif /*THROTTLE_H_*/
