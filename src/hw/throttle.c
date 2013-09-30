#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "throttle.h"
#include "i2c.h"
#include "site.h"
#include "../utils/utils.h"

/*static int steps[11] = { 0xFF, 0xED, 0xDF, 0xDE, 0xDC, 0xBF, 0xBE, 0x7F, 0x7E,
 0x9F, 0x8F };*/

static int steps[11] = { 0xFF, 0xF8, 0xF7, 0xF6, 0xF3, 0xEE, 0xEC, 0xE6, 0xDC,
    0xD3, 0x00 };

static float tts[12][2] = { { 1.90, 2.22 }, { 1.90, 2.22 }, { 1.90, 2.22 }, { 2.40, 2.80 },
    { 3.70, 4.00 }, { 5.00, 5.30 }, { 5.7, 6.1 }, { 6.8, 7.2}, { 7.8, 8.1 }, { 9.1,
        9.4 }, { 9.9, 10.2 }};

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
    if (val != 0) {
      th->mode = 1;
    } else {
      th->mode = 0;
    }
    return 1;
  } else {
    // wrong value
    // неправильное значение
    return 0;
  }
}

int i2c_get_th_data(int addr) {

  char buf[2];

  i2cOpen();

  if (ioctl(g_i2cFile, I2C_SLAVE, addr) < 0) {
    printf("Failed to acquire bus access and/or talk to slave.\n");
  }

  int k = read(g_i2cFile, buf, 2);

  i2cClose();

  int kl, kl1;
  kl = (int *) buf[0];
  //printf("1 kl = %d\n", kl);
  kl = kl * 256;
  //printf("2 kl = %d\n", kl);
  kl1 = (int *) buf[1];
  kl = kl + kl1;
  //printf("3 kl = %d kl1 = %d buf[1] = %d\n", kl, kl1, buf[1]);
  //printf("Проверка сдвига %d %x\n", kl, kl);

  site->th->position_adc = kl / 190.f;

  //printf("ЗНАЧ: %f 0x[%0x]\n", site->th->position_adc);
  int step = pos_to_step(site->th->position_adc);

  return step;
}

int pos_to_step(float pos) {

  int step = -5, i, j;
  //site->th->position;

  //printf("pos = %f,  position = %d \n", pos, site->th->position);
  if (tts[site->th->position][0] <= pos && tts[site->th->position][1] >= pos) {

    return site->th->position;

  }
  return -1;

  //for (i = 0; i < 11; i++) {
  //  for (j = 0; j < 2; j = j + 2) {
  //    if ((pos >= tts[i][j]) && (pos <= tts[i][j + 1])) {
  //      //printf("Значение между %d - %d \n", tts[i][j],tts[i][j+1]);
  //      step = i;
  //      break;
  //    }
  //  }
  //}

  //return step;
}

void throttle_free() {
  //TODO: очистим ресурсы памяти
}

Throttle* throttle_new() {
  Throttle* th = malloc(sizeof(Throttle));
  th->mode = 0;
  th->exist = 1; // Проверка наличия заслонки

  th->set_position = set_position;

  return th;
}

void test_throttle() {

  int i, step;
  char *buf;

  i2cOpen();

  char *a_th_adc = getStr(site->cfg, (void *) "a_throttle_adc");

  printf("Cчитаем данные\n");
  int tacho1, tacho2, th_r;

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

