#include <stdio.h>
#include <time.h>

#include "lcd.h"
#include "keyboard.h"
#include "menu.h"
#include "../hw/site.h"
#include "../config/config.h"
#include "ui.h"

/*
 *
 *
 *
 */
int dmode; // Мы либо в режиме редактирования 1, либо в режиме вывода 0
int omode; // Режим вывода
int ddiff; // задержка вывода
time_t time_start;

/*
 *
 *
 *
 */
int cpu_load() {
  return 1;
}

/*
 *
 *
 *
 */
int mem_total() {
  return 1;
}

/*
 *
 *
 *
 */
int uptime() {
  return 1;
}

/*
 *
 *
 *
 */
char* net_address() {
  return ("0.0.0.0");
}

void disp(Disp* lcd){

  char tmp_time[50], tmp_temp_out[50], tmp_temp_in[50], tmp_temp_mix[50], tmp_temp_evapor1[50], tmp_temp_evapor2[50];

  time_t rawtime;
  struct tm * timeinfo;
  printf("режим %d\n", omode);

  if (omode == 1) {
    // экран 1 - темп сайта , улицы
    printf("экран1\n");
    time(&rawtime);
    printf("время\n");
    timeinfo = localtime(&rawtime);
    printf("сделали структуру\n");
    sprintf(tmp_time, "%2d/%02d  %2d:%02d:%02d", timeinfo->tm_mday, 1 + timeinfo->tm_mon, timeinfo->tm_hour, timeinfo->tm_min,
        timeinfo->tm_sec);

    if (site->temp_out != -100.0) {
      sprintf(tmp_temp_out, "Улица  = %2.2f°C", site->temp_out);
    } else {
      sprintf(tmp_temp_out, "Улица  =  Ошибка");
    }
    //printf("2\n");
    if (site->temp_in != -100.0) {
      sprintf(tmp_temp_in, "Сайт   = %2.2f°C", site->temp_in);
    } else {
      sprintf(tmp_temp_in, "Сайт   =  Ошибка");
    }
    //printf("3\n");
    //printf("0\n");
    lcd_line(lcd, tmp_temp_out, 0);
    //printf("1 %s \n", tmp_temp_in);
    lcd_line(lcd, tmp_temp_in, 1);
    //printf("2\n");
    lcd_line(lcd, "Состояние работы", 2);
    //printf("3\n");
    lcd_line(lcd, tmp_time, 3);

    printf("экран1 конец\n");
  }

  //экран 2 - конд, конд, миксер, время
  if (omode == 2) {
    printf("экран2\n");
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    sprintf(tmp_time, "%2d/%02d  %2d:%02d:%02d", timeinfo->tm_mday, 1 + timeinfo->tm_mon, timeinfo->tm_hour, timeinfo->tm_min,
        timeinfo->tm_sec);

    if (site->temp_evapor1 != -100.0) {
      sprintf(tmp_temp_evapor1, "Конд1  = %2.2f°C", site->temp_evapor1);
    } else {
      sprintf(tmp_temp_evapor1, "Конд1  =  Ошибка");
    }

    if (site->temp_evapor2 != -100.0) {
      sprintf(tmp_temp_evapor2, "Конд2  = %2.2f°C", site->temp_evapor2);
    } else {
      sprintf(tmp_temp_evapor2, "Конд2  =  Ошибка");
    }

    if (site->temp_mix != -100.0) {
      sprintf(tmp_temp_mix, "Миксер = %2.2f°C", site->temp_mix);
    } else {
      sprintf(tmp_temp_mix, "Миксер =  Ошибка");
    }
    lcd_line(lcd, tmp_temp_evapor1, 0);
    lcd_line(lcd, tmp_temp_evapor2, 1);
    lcd_line(lcd, tmp_temp_mix, 2);
    lcd_line(lcd, tmp_time, 3);
    printf("экран2 конец\n");
  }

  if (omode == 3) {

    //экран - обороты вент, вент2, заслонка полож

  }
}


/*
 *
 *
 *
 */
void run_ui(Site* site) {

  printf("режим UI\n");

  printf("инициализируем экран\n");
  int addr_lcd = strtol(getStr(site->cfg, (void *) "a_lcd"), (char **) NULL, 16);
  printf("addr: 0x%x\n", addr_lcd);
  Disp* lcd = lcd_new(addr_lcd);

  printf("инициализация экрана\n");
  init(lcd);

  printf("инициализируем клавиатуру\n");
  int addr_kb = strtol(getStr(site->cfg, (void *) "a_keyb"), (char **) NULL, 16);
  KB* kb = kb_new(addr_kb);

  printf("---\n");
  printf("инициализируем меню\n");
   // init_menu();

  //printf("Текущий пункт меню номер %d \n", menu->curr->id);

  sleep(2);

  int click = 0;
  mnmode = 0;
  dmode = 0;
  omode = 1;
  ddiff = 2;
  time_start = time(NULL);

  printf("Начнем цикл работы UI\n");
  while(1) {
    printf("отслеживаем переход в меню\n");
    //click = readKeys(kb);
    //if ((click != 0) && (click != 255))   // 0, 255 ошибка чтения
    //  onKeyClicked(lcd, click);


    //disp(lcd);
    //sleep(1);
    if ((time(NULL) - time_start) >= ddiff){
      printf("выводим на экран показатели датчиков\n");
      if (mnmode == 0)
        disp(lcd);
      printf("после disp\n");
      time_start = time(NULL);
      printf("после time\n");
    }
  }

}

