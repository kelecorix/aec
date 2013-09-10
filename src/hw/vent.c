#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "vent.h"
#include "i2c.h"

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
    // TODO: Управляем оборудованием
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
    // TODO: Управляем оборудованием
    return 1;
  } else {
    // wrong value
    // неправильное значение
    return 0;
  }
}

Vent* vent_new() {
  Vent* vent = malloc(sizeof(Vent));
  vent->mode = 0;
  vent->error = NOERROR;

  vent->set_mode = set_mode;
  vent->set_turns = set_turns;
  return vent;
}
