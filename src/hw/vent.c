#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "vent.h"
#include "i2c.h"
#include "site.h"

static int steps[11] = { 0xFF, 0xF8, 0xF7, 0xF6, 0xF3, 0xEE, 0xEC, 0xE6, 0xDC, 0xD3, 0x00 };

// Таблицы преобразования обороты в шаг
static int tts1[11][2] = { { 70, 120 }, { 240, 300 }, { 400, 480 }, { 520, 600 }, { 750, 820 }, { 890, 950 }, { 980, 1200 }, { 980, 1200 },
    { 1300, 1360 }, { 1200, 1600 }, { 1200, 1600 } };

static int tts2[11][2] = { { 70, 120 }, { 300, 350 }, { 420, 480 }, { 550, 590 }, { 610, 680 }, { 730, 770 }, { 780, 820 }, { 830, 870 }, {
    830, 870 }, { 800, 1300 }, { 800, 1300 } };

void vent_free() {
  //TODO: очистим ресурсы памяти

}

// Установим количество оборотов, 11 шагов управлен  ия скоростью
int set_step(Vent* vent, int val) {

  if (val >= 0 && val <= 11) {
    i2cOpen();
    int addr;
    //printf("Изменим сост. вент\n");
    if (vent->type == 0)
      addr = strtol(getStr(site->cfg, (void *) "a_vent_in"), NULL, 16);
    else
      addr = strtol(getStr(site->cfg, (void *) "a_vent_out"), NULL, 16);
    printf("Управляем регистром, адрес %d, значение %d, %d \n", addr, val, steps[val]);
    i2cSetAddress(addr);
    set_i2c_register(g_i2cFile, addr, 0, steps[val]);
    vent->step = val;
    int pre_mode = vent->mode; // для расчета моточасо
    if (val != 0) {
      vent->mode = 1;
    } else {
      vent->mode = 0;
    }

    send_moto(vent, pre_mode);

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

  if (site->tacho1_t == 0) {
    site->tacho2_t = 0;
  }
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
    if ((turns >= tts1[site->vents[0]->step][0]) && (turns <= tts1[site->vents[0]->step][1])) {
      step = site->vents[0]->step;
    }
  }
  if (type == 1) {
    if ((turns >= tts2[site->vents[1]->step][0]) && (turns <= tts2[site->vents[0]->step][1])) {
      step = site->vents[1]->step;
    }
  }

  return step;
}

static void send_moto(Vent* vent, int pre_mode) {

  if (vent->mode == pre_mode)
    return;

  if (vent->mode == 1) {
    vent->moto_start = time(NULL);
    log3("ac.c: Включим ВЕНТ_%d\n", vent);
  } else {
    vent->moto_stop = time(NULL);
    log3("Моточасы VENT_%d %d %d %d\n", vent->type, (vent->moto_stop - vent->moto_start), vent->moto_stop, vent->moto_start);
    logD(site->logger->dataLOG, 0, "Моточасы VENT_%d %d", vent->type, (vent->moto_stop - vent->moto_start));
  }
}

void test_vents() {

  int i, step, tacho1, tacho2, step1, step2;
  char *buf;

  char *a_tacho_in = getStr(site->cfg, (void *) "a_tacho_flow_in");
  char *a_tacho_out = getStr(site->cfg, (void *) "a_tacho_flow_out");

  for (i = 0; i < 11; i++) {

    site->vents[0]->set_step(site->vents[0], i);
    site->vents[1]->set_step(site->vents[1], i);

    sleep(30);

    tacho1 = i2c_get_tacho_data(site->vents[0], strtol(a_tacho_in, NULL, 16));
    tacho2 = i2c_get_tacho_data(site->vents[1], strtol(a_tacho_out, NULL, 16));

    step1 = turns_to_step(tacho1, site->vents[0]->type);
    step2 = turns_to_step(tacho2, site->vents[1]->type);

    printf("Шаг %d: tахо1 %d, tахо2 %d, \n", i, tacho1, tacho2);
    printf("Шаг %d: vent1 %d, vent2 %d\n", i, step1, step2);
  }
}

Vent* vent_new() {
  Vent* vent = malloc(sizeof(Vent));
  vent->mode = 0;
  vent->error = NOERROR;

  vent->set_step = set_step;
  return vent;
}
