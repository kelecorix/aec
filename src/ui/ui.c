#include <stdio.h>
#include <time.h>
#include "ui.h"
#include "lcd.h"
#include "keyboard.h"
#include "menu.h"
#include "../hw/site.h"
#include "../config/config.h"

int dmode; // Мы либо в режиме редактирования 1, либо в режиме вывода 0

void run_ui(Site* site) {

  // инициализируем экран
  int addr_lcd = strtol(getStr(site->cfg, (void *) "a_lcd"), (char **) NULL, 16);
  LCD* lcd = lcd_new(addr_lcd);
  lcd->init(lcd);
  lcd->reset(lcd);

  // инициализируем клавиатуру
  int addr_kb = strtol(getStr(site->cfg, (void *) "a_keyb"), (char **) NULL, 16);
  KB* kb = kb_new(addr_kb);

  // инициализируем меню
  //init_menu();

  // установим обработчик нажатия
  //setHandler(MSG_MENU_SELECT, &selectMenu);

  sleep(1);

  dmode = 0;
  while (1) {

  // отслеживаем переход в меню

  // выводим на экран показатели датчиков




  }
}


int cpu_load() {
  return 1;
}

int mem_total() {
  return 1;
}

int uptime() {
  return 1;
}

char* net_address() {
  return ("0.0.0.0");
}

