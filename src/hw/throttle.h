#ifndef THROTTLE_H_
#define THROTTLE_H_

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "i2c.h"

/*
 *
 *
 *
 */
typedef struct Throttle {

  int mode; // 0 - сайт, 1 улица
  float position_adc;
  int position; // 0 -255
  int exist; // 1 - заслонка есть, 0 - заслонка отсутствует
  long time_work;
  time_t time_start;
  time_t time_stop;

  int (*set_mode)(struct Throttle*, int val);
  int (*set_position)(struct Throttle*, int val);

} Throttle;

Throttle* throttle_new();
int i2c_get_th_data(int addr);
int pos_to_step(float pos);
void test_throttle();


#endif /*THROTTLE_H_*/
