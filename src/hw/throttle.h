#ifndef THROTTLE_H_
#define THROTTLE_H_

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "i2c.h"

// Заслонка
typedef struct Throttle {

  int mode; // 0 - сайт, 1 улица
  int position; // 0 -255
  int exist; // 1 - заслонка есть, 0 - заслонка отсутствует
  long time_work;
  time_t time_start;
  time_t time_stop;

  int steps[10] = {0xFF, 0xED, 0xDF, 0xDE, 0xDC, 0xBF, 0xBE, 0x7F, 0x7E, 0x9F,
         0x8F};

  int (*set_mode)(struct Throttle*, int val);
  int (*set_position)(struct Throttle*, int val);

} Throttle;

#endif /*THROTTLE_H_*/
