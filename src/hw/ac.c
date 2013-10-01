#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "ac.h"
#include "i2c.h"
#include "site.h"

void ac_free() {
//TODO: очистим ресурсы памяти
}

/* Функция старта кондиционера
 */
int ac_start(AC* ac) {
  // Получим текущее время, зафиксируем как время старта
  ac->time_start = time(NULL);

  // Изменим режим работы
  ac->set_mode(ac, 1);

  return 1;
}

/* Выполним стоп кондционера
 * 
 */
int ac_stop(AC* ac) {
  // Получим текущее время, зафиксируем как время остановки
  ac->time_stop = time(NULL);

  // Измени режим работы
  ac->set_mode(ac, 0);
  return 1;
}

/*
 * Рассчитаем время работы
 */
double ac_time_work(AC* ac) {
  double seconds = difftime(ac->time_start, ac->time_stop);
  return seconds;
}

/*
 * Рассчитаем моточасы
 */
long ac_moto_work(AC* ac) {
  int time = ac->ac_time_work;
  long moto = time * 1; // time * motoparameter 
  return moto;
}

/*
 * Изменим режим кондиционирования
 */
static int set_mode(AC* ac, int val) {

  i2cOpen();

  int addr, value, bit;
  addr = strtol(getStr(site->cfg, "a_relay"), NULL, 16);
  unsigned char rvalue;
  char buf[1];

  if (ioctl(g_i2cFile, I2C_SLAVE, addr) < 0)
    printf("Failed to acquire bus access and/or talk to slave.\n");

  if (read(g_i2cFile, buf, 1) != 1)
    log1("set_mode: Error reading from i2c\n");

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
    send_moto_ac(ac);
    i2cClose();
    return 1;
  } else {
    // неправильное значение
    return 0;
  }
}

/* Внутренняя функция передачи данных моточасов
 *
 */
static void send_moto_ac(AC* ac) {

  if (ac->mode == 1) {
    ac->moto_start = time(NULL);
    log3("ac.c: Включим КОНД_%d\n", ac->num);
  } else {
    ac->moto_stop = time(NULL);
    log3("Моточасы KOND_%d %d %d %d\n", ac->num, (ac->moto_stop - ac->moto_start), ac->moto_stop, ac->moto_start);
    logD(site->logger->dataLOG, 0, "Моточасы KOND_%d %d", ac->num, (ac->moto_stop - ac->moto_start));
  }
}

AC* ac_new(int i) {
  AC* ac = malloc(sizeof(AC));
  ac->mode = 0;
  ac->error = NO_ERROR;

  ac->ac_start = ac_time_work;
  ac->ac_stop = ac_stop;
  ac->ac_time_work = ac_start;
  ac->ac_moto_work = ac_moto_work;
  ac->set_mode = set_mode;
  ac->num = i;

  return ac;
}
