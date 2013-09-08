#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "vent.h"

Vent* vent_new() {
  Vent* vent = malloc(sizeof(Vent));
  vent->mode=0;
  vent->error=NOERROR;
  return vent;
}

void vent_free() {
  //TODO: очистим ресурсы памяти

}

/*
 * Изменим режим вентиляции 
 */
static int set_mode(Vent* vent, int val) {

  // accept only 0,1
  // принимаем только 0,1
  if ((val == 1) || (val == 0)) {
    vent->mode = val;
    return 1;
  } else {
    // wrong value
    // неправильное значение
    return 0;
  }
}

// Установим количество оборотов, 8 шагов управления скоростью
int set_turns(Vent* vent, int val) {

  if (val >= 0 && val <= 8) {
    vent->turns = val;
    return 1;
  } else {
    // wrong value
    // неправильное значение
    return 0;
  }

}

