#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "ac.h"

AC* ac_new() {
  AC* ac = malloc(sizeof(AC));
  ac->mode=0;
  ac->error=NO_ERROR;
  return ac;
}

void ac_free() {

  //TODO: очистим ресурсы памяти
}
/*
 * Выполним старт кондционера
 * 
 */
int ac_start(AC* ac) {

  // Пet current time, save as start time 
  // Получим текущее время, зафиксируем как время старта
  ac->time_start = time(NULL);

  // Сhange mode
  // Изменим режим работы
  ac->mode = 1;

  return 1;

}

/*
 * Выполним стоп кондционера
 * 
 */
int ac_stop(AC* ac) {

  // Get current time, save as stop time
  // Получим текущее время, зафиксируем как время остановки
  ac->time_stop = time(NULL);

  // Сhange mode
  // Измени режим работы
  ac->mode = 0;

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
