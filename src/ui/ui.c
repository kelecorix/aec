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



void disp(lcd, curr){

  char tmp_value[50], tmp_time[50], tmp_temp_out[50], tmp_temp_in[50],
       tmp_temp_mix[50], tmp_temp_evapor1[50], tmp_temp_evapor2[50], buffer[200];
   int tek_znach = 25;
   time_t rawtime;
   struct tm * timeinfo;

  switch (curr){
  case 0 :
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    sprintf(tmp_time, "%2d/%02d  %2d:%02d:%02d", timeinfo->tm_mday, 1 + timeinfo->tm_mon, timeinfo->tm_hour, timeinfo->tm_min,
        timeinfo->tm_sec);

    if (site->temp_out != -100.0) {
      sprintf(tmp_temp_out, "Улица  = %2.2f°C", site->temp_out);
    } else {
      sprintf(tmp_temp_out, "Улица  =  Ошибка");
    }
    if (site->temp_in != -100.0) {
      sprintf(tmp_temp_in, "Сайт   = %2.2f°C", site->temp_in);
    } else {
      sprintf(tmp_temp_in, "Сайт   =  Ошибка");
    }

    lcd_line(lcd, tmp_temp_out, 0);
    lcd_line(lcd, tmp_temp_in, 1);
    lcd_line(lcd, "Состояние работы", 2);
    lcd_line(lcd, tmp_time, 3);

    break;
  case 2000 :
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

  }
}

/*
 *
 *
 *
 */
void run_ui(Site* site) {

  printf("режим UI\n");
  // инициализируем экран
  int addr_lcd = strtol(getStr(site->cfg, (void *) "a_lcd"), (char **) NULL, 16);
  printf("addr: 0x%x\n", addr_lcd);
  Disp* lcd = lcd_new(addr_lcd);


  init(lcd);
  //reset(lcd);

  // инициализируем клавиатуру
  // int addr_kb = strtol(getStr(site->cfg, (void *) "a_keyb"), (char **) NULL, 16);
  // KB* kb = kb_new(addr_kb);

  // инициализируем меню
  //init_menu();

  sleep(1);

  int click,out_curr=0, menu_curr=0;
  dmode = 0;
  printf("цикл работы UI\n");
  while (1) {

    // отслеживаем переход в меню
    click = readKeys();
    if (click != NULL)
      onKeyClicked(click);

    if (out_curr == 0) {
      if (menu_curr == 0 || menu_curr == 2000) {
        if (menu_curr == 0) {
          menu_curr = 2000;
        } else {
          menu_curr = 0;
        }
      }
      out_curr = 40;
    }

    // выводим на экран показатели датчиков
    if (mnmode == 0)
      disp(lcd, menu_curr);

  }
}

