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

static int tts[11][2] = { { 245, 265 }, { 327, 347 }, { 413, 453 },
    { 512, 532 }, { 610, 630 }, { 681, 701 }, { 780, 800 }, { 817, 837 }, { 916,
        936 }, { 1049, 1069 }, { 1133, 1153 } };

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

  printf("заслонка считано 0x%X 0x%X \n", buf[0], buf[1]);

  int num = 0;

  num = buf[1];
  printf("buf[1] был 0x%X \n", num);
  int rev = 0x0;
  int digit = 0x0;
  while (num != 0x0) {
    digit = num % 0x10;
    rev = rev * 0x10 + digit;
    num = num / 0x10;
  }

  printf("buf[1] стал 0x%X \n", rev);
  buf[1] = rev;
  buf[2] = 0x0;
  buf[3] = 0x0;
  revS(buf);
  printf("до приведения 0x%X 0x%X \n", buf[0], buf[1]);

  int value = *((int *) buf);
  //printf("Буферы: %x, %x", buf[0], buf[1]);

  printf("После приведения: 0x%X, %d \n", value, value);
  site->th->position_adc = (int) ((value / 190) * 100);
  printf("ЗНАЧ: %d 0x[%0x]\n", site->th->position_adc);
  int step = pos_to_step(value);

  return step;
}

int pos_to_step(int pos) {

  int step = -5, i, j;
  //site->th->position;

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

