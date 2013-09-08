#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "throttle.h"

Throttle* throttle_new() {
  Throttle* th = malloc(sizeof(Throttle));
  th->mode = 0;
  return th;
}

void throttle_free() {
  //TODO: очистим ресурсы памяти
}

/*
 * Изменим режим заслонки 
 */
static int set_mode(Throttle* th, int val) {

  // accept only 0,1
  // принимаем только 0,1
  if ((val == 1) || (val == 0)) {
    th->position = val;
    return 1;
  } else {
    // wrong value
    // неправильное значение
    return 0;
  }
}

int set_position(Throttle* th, int val) {

  if (val >= 0 && val <= 255) {
    th->position = val;
    return 1;
  } else {
    // wrong value
    // неправильное значение
    return 0;
  }

}

