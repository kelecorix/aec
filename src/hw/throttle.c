#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "throttle.h"
#include "i2c.h"
#include "site.h"
#include "../utils/utils.h"

/* Значения шагов управления положением заслонки
 * в соответствии со спецификацией оборудования
 */
static int steps[11] = { 0xFF, 0xF8, 0xF7, 0xF6, 0xF3, 0xEE, 0xEC, 0xE6, 0xDC, 0xD3, 0x00 };

/* Таблица преобразования оборотов
 * приточного вентилятора в шаг управления
 * Сама таблица содержится в конфигурационном файле
 */
static float tts[12][2];

/* Установка позиции заслонки
 *
 *
 *
 */
int set_position(Throttle* th, int val) {

  i2cOpen();
  if (val >= 0 && val <= 10) {
    int addr;
    addr = strtol(getStr(site->cfg, (void *) "a_throttle"), NULL, 16);
    printf("Заслонка set_position, адрес %x, значение %d, %d\n", addr, val, steps[val]);
    set_i2c_register(g_i2cFile, addr, steps[val], steps[val]);
    th->position = val;
    i2cClose();
    if (val != 0) {
      th->mode = 1;
    } else {
      th->mode = 0;
    }
    return 1;
  } else {
    // неправильное значение
    return 0;
  }
}

/*
 *
 *
 *
 */
int i2c_get_th_data(int addr) {

  char buf[2];

  i2cOpen();

  if (ioctl(g_i2cFile, I2C_SLAVE, addr) < 0) {
    printf("Failed to acquire bus access and/or talk to slave.\n");
  }

  read(g_i2cFile, buf, 2);

  i2cClose();

  int kl, kl1;
  kl = (int *) buf[0];
  kl = kl * 256;
  kl1 = (int *) buf[1];
  kl = kl + kl1;

  site->th->position_adc = kl / 190.f;

  //printf("ЗНАЧ: %f 0x[%0x]\n", site->th->position_adc);
  int step = pos_to_step(site->th->position_adc);

  return step;
}

/*
 *
 *
 *
 */
int pos_to_step(float pos) {

  if (tts[site->th->position][0] <= pos && tts[site->th->position][1] >= pos)
    return site->th->position;

  return -1;
}

/*
 *
 *
 *
 */
void throttle_free() {
  //TODO: очистим ресурсы памяти
}

/*
 *
 *
 *
 */
Throttle* throttle_new() {
  Throttle* th = malloc(sizeof(Throttle));
  th->mode = 0;
  th->exist = 1; // Проверка наличия заслонки

  th->set_position = set_position;

  return th;
}

/*
 *
 *
 *
 */
void test_throttle() {

  int i, step;

  i2cOpen();

  char *a_th_adc = getStr(site->cfg, (void *) "a_throttle_adc");

  printf("Cчитаем данные\n");

  site->th->set_position(site->th, 0);

  sleep(10);

  printf("Прямой ход\n");
  for (i = 0; i < 11; i++) {

    site->th->set_position(site->th, i);

    sleep(30);

    step = i2c_get_th_data(strtol(a_th_adc, NULL, 16));
    printf("Шаг %d: заслонка в %d [ADC %f]\n", i, step, site->th->position_adc);

  }

  printf("Обратный ход\n");
  for (i = 11; i >= 0; i--) {

    site->th->set_position(site->th, i);

    sleep(30);

    step = i2c_get_th_data(strtol(a_th_adc, NULL, 16));
    printf("Шаг %d: заслонка в %d [ADC %f]\n", i, step, site->th->position_adc);

  }

  i2cClose();
}

