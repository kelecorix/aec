#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "vent.h"
#include "i2c.h"
#include "site.h"

static int steps[11] = { 0xFF, 0xED, 0xDF, 0xDE, 0xDC, 0xBF, 0xBE, 0x7F, 0x7E,
    0x9F, 0x8F };

void vent_free() {
  //TODO: очистим ресурсы памяти

}

// Установим количество оборотов, 8 шагов управлен  ия скоростью
int set_turns(Vent* vent, int val) {

  if (val >= 0 && val <= 10) {
    i2cOpen();
    int addr;
    //printf("Изменим сост. вент\n");
    if (vent->type == 0)
      addr = strtol(getStr(site->cfg, (void *) "a_vent_in"), NULL, 16);
    else
      addr = strtol(getStr(site->cfg, (void *) "a_vent_out"), NULL, 16);
    printf("Управляем регистром, адрес %d, значение %d, %d \n", addr, val,
        steps[val]);
    i2cSetAddress(addr);
    set_i2c_register(g_i2cFile, addr, 0, steps[val]);
    vent->turns = val;
    if(val!=0){
      vent->mode = 1;
    }else{
      vent->mode = 0;
    }
    i2cClose();
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
