#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "i2c.h"
#include "site.h"
#include "../log/file_logger.h"
#include "../config/config.h"
#include "ac.h"

void ac_free() {
//TODO: очистим ресурсы памяти
}

/* Внутренняя функция передачи данных моточасов
 *
 *
 *
 */
static void send_moto(AC* ac) {

  if (ac->mode == 1) {
    ac->moto_start = time(NULL);
    log_3("ac.c: Включим КОНД_%d\n", ac->num);
  } else {
    ac->moto_stop = time(NULL);
    log_3("Моточасы KOND_%d %d %d %d\n", ac->num, (ac->moto_stop - ac->moto_start), ac->moto_stop, ac->moto_start);
    logD(site->logger->dataLOG, 0, "Моточасы KOND_%d %d", ac->num, (ac->moto_stop - ac->moto_start));
  }
}

/*
 *
 *
 *
 */
static int set_mode(AC* ac, int val) {

  i2cOpen();

  int addr, value, bit;
  addr = strtol(getStr(site->cfg, "a_relay"), NULL, 16);
  char buf[1];

  if (ioctl(g_i2cFile, I2C_SLAVE, addr) < 0)
    log_4("Failed to acquire bus access and/or talk to slave.\n");

  if (read(g_i2cFile, buf, 1) != 1)
    log_4("set_mode: Error reading from i2c\n");

  value = (int) buf[0];

  if ((val == 1) || (val == 0)) {

    if (ac->num == 0)
      bit = 2;

    if (ac->num == 1)
      bit = 1;

    if (val == 1)
      value |= (1 << bit); // установим бит
    else
      value &= ~(1 << bit); // очистим бит

    //log4("Управляем регистром, адрес %x, значение %d, %x , бит %d , номер %d\n", addr, val, value, bit, ac->num);
    set_i2c_register(g_i2cFile, addr, value, value);

    ac->mode = val;
    send_moto(ac);
    i2cClose();
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
AC* ac_new(int i) {
  AC* ac = malloc(sizeof(AC));
  ac->mode = 0;
  ac->error = NO_ERROR;

  ac->set_mode = set_mode;
  ac->num = i;

  return ac;
}
