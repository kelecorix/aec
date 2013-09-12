#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "throttle.h"
#include "i2c.h"
#include "site.h"

static int steps[10] = { 0xFF, 0xED, 0xDF, 0xDE, 0xDC, 0xBF, 0xBE, 0x7F, 0x7E, 0x9F,
     0x8F};

/*
 * Изменим режим заслонки 
 */
static int set_mode(Throttle* th, int val) {

  i2cOpen();
  if ((val == 1) || (val == 0)) {
    int addr, value;
    addr = strtol(getStr(site->cfg, (void *) "a_throttle"), NULL, 16);
    if (val == 1)
      value = 8; // максимальное значение
    else
      value = 1; // минимальное значение
    printf("Управляем регистром, адрес %x, значение %d, %x\n", addr, val, value);
    set_i2c_register(g_i2cFile, addr, steps[value], steps[value]);
    th->position = val;
    i2cClose();
    return 1;
  } else {
    // wrong value
    // неправильное значение
    return 0;
  }
}

int set_position(Throttle* th, int val) {
  i2cOpen();
  if (val >= 0 && val <= 8) {
    int addr;
    addr = strtol(getStr(site->cfg, (void *) "a_throttle"), NULL, 16);
    set_i2c_register(g_i2cFile, addr, steps[val], steps[val]);
    th->position = val;
    i2cClose();
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
  th->exist = 1; // TODO: Проверка наличия заслонки

  th->set_mode = set_mode;
  th->set_position = set_position;

  return th;
}

