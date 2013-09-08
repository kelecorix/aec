#ifndef THROTTLE_H_
#define THROTTLE_H_

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

// Заслонка
typedef struct Throttle {

  int mode; // 0 - сайт, 1 улица
  int position; // 0 -255

  time_t time_start;
  time_t time_stop;
  long time_work;

  int (*set_mode)(struct AC*, int val);
  int (*set_position)(struct AC*, int val);

} Throttle;

#endif /*THROTTLE_H_*/
