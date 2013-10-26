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

/*
 *
 *
 *
 */
void run_ui() {

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

  init_menu();

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
    //printf("отслеживаем переход в меню\n");
    click = readKeys(kb);
    //printf("нажато %d\n", click);
    if ((click != 0) && (click != 255))   // 0, 255 ошибка чтения
      onKeyClicked(lcd, click);

    //disp(lcd);
    //sleep(1);
    if ((time(NULL) - time_start) >= ddiff){
      //printf("выводим на экран показатели датчиков\n");
      if (mnmode == 0)
        disp(lcd);
      //printf("после disp\n");
      time_start = time(NULL);
      //printf("после time\n");
    }
  }

}

