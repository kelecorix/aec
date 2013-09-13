#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "throttle.h"
#include "i2c.h"
#include "site.h"

static int steps[11] = { 0xFF, 0xED, 0xDF, 0xDE, 0xDC, 0xBF, 0xBE, 0x7F, 0x7E,
    0x9F, 0x8F };

int set_position(Throttle* th, int val) {

  i2cOpen();
  if (val >= 0 && val <= 10) {
    int addr;
    addr = strtol(getStr(site->cfg, (void *) "a_throttle"), NULL, 16);
    printf("Заслонка set_position, адрес %x, значение %d, %d\n", addr, val,
        steps[val]);
    set_i2c_register(g_i2cFile, addr, steps[val], steps[val]);
    th->position = val;
    i2cClose();
    if(val!=0){
        th->mode = 1;
      }else{
        th->mode = 0;
      }
    return 1;
  } else {
    // wrong value
    // неправильное значение
    return 0;
  }
}

void throttle_free() {
  //TODO: очистим ресурсы памяти
}

Throttle* throttle_new() {
  Throttle* th = malloc(sizeof(Throttle));
  th->mode = 0;
  th->exist = 1; // Проверка наличия заслонки

  th->set_mode = set_mode;
  th->set_position = set_position;

  return th;
}

