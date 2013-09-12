#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "ac.h"
#include "i2c.h"
#include "site.h"

void ac_free() {
//TODO: очистим ресурсы памяти
}
/*
 * Выполним старт кондционера
 * 
 */
int ac_start(AC* ac) {
  // Get current time, save as start time
  // Получим текущее время, зафиксируем как время старта
  ac->time_start = time(NULL);

  // Сhange mode
  // Изменим режим работы
  ac->set_mode(ac, 1);

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
  addr = 0b00111011;
  unsigned char rvalue;

  //прочитаем текущее состояние регистра
  if(get_i2c_register(g_i2cFile, addr, 0, &value)) {
    printf("Unable to get register!\n");
  }
  else {
    printf("Addr %d: %d (%x)\n", addr, (int)rvalue, (int)rvalue);
  }

  value = (int)rvalue;

  if ((val == 1) || (val == 0)) {
   printf("Изменим сост. кондиц\n");

    if(ac->num == 0){
      bit = 3;
    }
    if(ac->num == 1){
      bit = 2;
    }
    if(val==1)
      value |= (1 << bit); // максимальное значение
    else
      value &= ~(1 << bit) ; // минимальное значение

    printf("Управляем регистром, адрес %d, значение %d, %d \n", addr, val, value);
    set_i2c_register(g_i2cFile, addr, 0, value);
    ac->mode = val;
    i2cClose();
    return 1;
  } else {
    // wrong value
    // неправильное значение
    return 0;
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
  ac->num =i;

  return ac;
}
