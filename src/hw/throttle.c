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

char* i2c_get_th_data(int addr) {

  int value, bit;
  unsigned char rvalue, rvalue2;
  char buf[3];
  char *buf_ptr;

  i2cOpen();

//  if (get_i2c_register(g_i2cFile, addr, 0x48, &rvalue)) {
//    printf("Unable to get register!\n");
//  } else {
//    printf("First byte %x: %d (%x)\n", addr, (int) rvalue, (int) rvalue);
//  }
//
//  if (get_i2c_register(g_i2cFile, addr, 0x48, &rvalue2)) {
//    printf("Unable to get register!\n");
//  } else {
//    printf("Second byte %x: %d (%x)\n", addr, (int) rvalue2, (int) rvalue2);
//  }

  printf("Установим адрес\n");

  if (ioctl(g_i2cFile, I2C_SLAVE, addr) < 0) {
    printf("Failed to acquire bus access and/or talk to slave.\n");
  }

  printf("Попробуем считать адрес\n");

  int k = read(g_i2cFile, buf, 2);

  printf("заслонка считано %d, %d \n",buf[0], buf[1]);

  i2cClose();

  buf_ptr = &buf;

  return buf_ptr;

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

