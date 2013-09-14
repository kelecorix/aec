#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "vent.h"
#include "i2c.h"
#include "site.h"

static int steps[11] = { 0xFF, 0xED, 0xDF, 0xDE, 0xDC, 0xBF, 0xBE, 0x7F, 0x7E,
    0x9F, 0x8F };

static int tts1[11][2] = { { 120, 130 }, { 119, 120 }, { 410, 420 },
    { 620, 640 }, { 780, 810 }, { 780, 810 }, { 910, 930 }, { 990, 1100 }, {
        1190, 1210 }, { 1320, 1340 }, { 1490, 1510 } };

static int tts2[11][2] = { { 120, 130 }, { 290, 315 }, { 490, 510 },
    { 621, 641 }, { 655, 675 }, { 740, 760 }, { 740, 760 }, { 780, 810 }, { 840,
        860 }, { 910, 930 }, { 910, 930 } };

void vent_free() {
  //TODO: очистим ресурсы памяти

}

// Установим количество оборотов, 8 шагов управлен  ия скоростью
int set_turns(Vent* vent, int val) {

  if (val >= 0 && val <= 11) {
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
    if (val != 0) {
      vent->mode = 1;
    } else {
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

int i2c_get_tacho_data(Vent* v, int addr) {

  int value, bit;
  unsigned char rvalue;

  i2cOpen();

  if (get_i2c_register(g_i2cFile, addr, 0x02, &rvalue)) {
    printf("Unable to get register!\n");
  } else {
    printf("Addr %x: %d (%x)\n", addr, (int) rvalue, (int) rvalue);
  }

  i2cClose();

  // Преобразуем количество импульсов в обороты
  float turns = ((1000 / (float) rvalue) * 60) / 5;

  // Преобразуем обороты в номер шага
  int turns_step = turns_to_step((int) turns, v->type);

  return turns;
}

int turns_to_step(int turns, int type) {

  int step, i, j;

  if (type == 1) {

    for (i = 0; i < 11; i++) {
      for (j = 0; j < 2; j = j + 2) {
        if ((turns > tts1[i][j]) && (turns < tts1[i][j + 1])) {
          step = i;
          break;
        }
      }
    }
  } else {
    for (i = 0; i < 11; i++) {
      for (j = 0; j < 2; j = j + 2) {
        if ((turns > tts2[i][j]) && (turns < tts2[i][j + 1])) {
          step = i;
          break;
        }
      }
    }
  }

  return step;
}

Vent* vent_new() {
  Vent* vent = malloc(sizeof(Vent));
  vent->mode = 0;
  vent->error = NOERROR;

  vent->set_turns = set_turns;
  return vent;
}
