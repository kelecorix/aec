#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "vent.h"
#include "i2c.h"
#include "site.h"

//static int steps[11] = { 0xFF, 0xED, 0xDF, 0xDE, 0xDC, 0xBF, 0xBE, 0x7F, 0x7E,
//    0x9F, 0x8F };

static int steps[11] = { 0xFF, 0xF8, 0xF7, 0xF6, 0xF3, 0xEE, 0xEC, 0xE6, 0xDC,
    0xD3, 0x00 };

// Таблицы преобразования обороты в шаг
static int tts1[11][2] = { { 120, 130 }, { 119, 120 }, { 410, 420 },
    { 620, 640 }, { 780, 810 }, { 780, 810 }, { 910, 930 }, { 990, 1100 }, {
        1190, 1210 }, { 1320, 1340 }, { 1490, 1510 } };

static int tts2[11][2] = { { 120, 130 }, { 290, 315 }, { 490, 510 },
    { 621, 641 }, { 655, 675 }, { 740, 760 }, { 740, 760 }, { 780, 810 }, { 840,
        860 }, { 910, 930 }, { 910, 1100 } };

void vent_free() {
  //TODO: очистим ресурсы памяти

}

// Установим количество оборотов, 8 шагов управлен  ия скоростью
int set_step(Vent* vent, int val) {

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
    vent->step = val;
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

void i2c_get_tacho(int addr0, int addr1) {

  unsigned char rvalue0, rvalue1;

  i2cOpen();

  get_i2c_register(g_i2cFile, addr0, 0x02, &rvalue0);
  get_i2c_register(g_i2cFile, addr1, 0x02, &rvalue1);

  i2cClose();

  // Преобразуем количество импульсов в обороты
  site->tacho1_t = (int) (((1000 / (float) rvalue0) * 60) / 5);
  site->tacho2_t = (int) (((1000 / (float) rvalue1) * 60) / 5);

  if (site->tacho1_t < 100)
    site->tacho1_t = 0;

  if (site->tacho2_t < 100)
    site->tacho2_t = 0;

  // Преобразуем количество оборотов в шаг
  site->tacho1 = turns_to_step(site->tacho1_t, site->vents[0]->type);
  site->tacho2 = turns_to_step(site->tacho2_t, site->vents[1]->type);

}

int i2c_get_tacho_data(Vent* v, int addr) {

  int value, bit;
  unsigned char rvalue;

  i2cOpen();

  if (get_i2c_register(g_i2cFile, addr, 0x02, &rvalue)) {
    printf("Unable to get register!\n");
  } else {
    //printf("Addr %x: %d (%x)\n", addr, (int) rvalue, (int) rvalue);
  }

  i2cClose();

  // Преобразуем количество импульсов в обороты
  float turns = ((1000 / (float) rvalue) * 60) / 5;

  return turns;
}

int i2c_get_tacho_step(Vent* v, int addr) {

  int value, bit;
  unsigned char rvalue;

  i2cOpen();

  if (get_i2c_register(g_i2cFile, addr, 0x02, &rvalue)) {
    printf("Unable to get register!\n");
  } else {
    //printf("Addr %x: %d (%x)\n", addr, (int) rvalue, (int) rvalue);
  }

  i2cClose();

  // Преобразуем количество импульсов в обороты
  float turns = ((1000 / (float) rvalue) * 60) / 5;

  // Преобразуем обороты в номер шага
  int turns_step = turns_to_step((int) turns, v->type);

  return turns_step;
}

int turns_to_step(int turns, int type) {

  if (turns <= 100) {
    return 0;
  }

  int step = -5, i, j;

  if (type == 0) {
    //for (i = 0; i < 11; i++) {
    //  for (j = 0; j < 2; j = j + 2) {
    //if ((turns >= tts1[i][j]) && (turns <= tts1[i][j + 1])) {
    if ((turns >= tts1[site->vents[0]->step][0])
        && (turns <= tts1[site->vents[0]->step][1])) {
      step = site->vents[0]->step;
      //break;
    }
    //  }
    //}
  }
  if (type == 1) {
    //for (i = 0; i < 11; i++) {
    //  for (j = 0; j < 2; j = j + 2) {
    //if ((turns >= tts2[i][j]) && (turns <= tts2[i][j + 1])) {
    if ((turns >= tts2[site->vents[1]->step][0])
        && (turns <= tts2[site->vents[0]->step][1])) {
      step = site->vents[1]->step;
      //break;
    }
    //  }
    //}
  }

  return step;
}

void test_vents() {

  int i, step, tacho1, tacho2, th_r;
  char *buf;

  char *a_tacho_in = getStr(site->cfg, (void *) "a_tacho_flow_in");
  char *a_tacho_out = getStr(site->cfg, (void *) "a_tacho_flow_out");

  for (i = 1; i <= 11; i++) {

    site->vents[0]->set_step(site->vents[0], i);
    site->vents[1]->set_step(site->vents[1], i);

    sleep(30);

    tacho1 = i2c_get_tacho_data(site->vents[0], strtol(a_tacho_in, NULL, 16));
    tacho2 = i2c_get_tacho_data(site->vents[1], strtol(a_tacho_out, NULL, 16));

    printf("Шаг %d: tахо1 %d, tахо2 %d, \n", i, tacho1, tacho2);

  }

}

Vent* vent_new() {
  Vent* vent = malloc(sizeof(Vent));
  vent->mode = 0;
  vent->error = NOERROR;

  vent->set_step = set_step;
  return vent;
}
