#include <stdio.h>
#include <time.h>

#include "ui.h"
#include "lcd.h"
#include "keyboard.h"
#include "../hw/site.h"
#include "../config/config.h"

void run_ui(Site* site) {

  printf("Running ui module\n");
  //прочитаем адрес из конфигурации
  char *key = "a_lcd";
  char *a_lcd = getStr(site->cfg, (void *) key);
  int addr = strtol(a_lcd, (char **) NULL, 16);
  printf("LCD addr = %d\n", addr);
  LCD* lcd = lcd_new(addr);

  int menu_curr = 0; // текущее меню
  int val_curr = 0;  // текущее значение
  int out_curr = 40; // текущий вывод, смеситель и кондиционер попеременно

  //int display_mode = mode_intro;
  display(site, lcd, 1000);

  //usleep(5000000);
  usleep(500000);

  //display_mode = mode_OSN;

  while (1) {

    /***********************************************************
     * Выводим по переменно температуру миксера или кондиционера
     * menu_curr == 0    - миксер
     * menu_curr == 2000 - кондиционер
     */

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

    display(site, lcd, menu_curr);

    key = "a_keyb";
    char *a_keyb = getStr(site->cfg, (void *) key);
    int kb_addr = strtol(a_keyb, (char **) NULL, 16);
    KB* kb = kb_new(kb_addr);

    //display_mode = keyboard();
    menu_curr = keyboard(menu_curr);
    //printf("%d\n",tek_menu);
    out_curr--;
    usleep(10000);
  }
}

void display(Site* site, LCD* lcd, int display_mode) {

  char tmp_value[50], tmp_time[50], tmp_temp_out[50], tmp_temp_in[50],
      tmp_temp_mix[50], tmp_temp_evapor1[50], tmp_temp_evapor2[50], buffer[200];
  //char* host;
  //int mem_tot, i_cpu_load, cpu_interval = 0;
  int tek_znach = 25;
  //long double b[7];
  time_t rawtime;
  struct tm * timeinfo;
  //-----
  //char *key;

  switch (display_mode) {
    case 1000:
      lcd_line(lcd, "     Меандр", 0);       // TODO: из конфига
      lcd_line(lcd, "FreCooling v.0.1", 1);  // TODO: из конфига
      lcd_line(lcd, "Raspberri PI I2C", 2);  // TODO: из конфига
      lcd_line(lcd, "rev.h 0.1", 3);         // TODO: из конфига
      break;
    case 0:
      time(&rawtime);
      timeinfo = localtime(&rawtime);
      //sprintf(tmp_value,"%2d%02d%2d %2d:%02d:%02d", timeinfo->tm_mday,1+timeinfo->tm_mon,1900+timeinfo->tm_year,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
      sprintf(tmp_time, "%2d/%02d  %2d:%02d:%02d", timeinfo->tm_mday,
          1 + timeinfo->tm_mon, timeinfo->tm_hour, timeinfo->tm_min,
          timeinfo->tm_sec);
      //0xEF градус
      //TODO: добавить вывод всех необходимых параметров
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
      //lcd_line("Улица  = 20.00°C",0);
      lcd_line(lcd, tmp_temp_out, 0);
      lcd_line(lcd, tmp_temp_in, 1);
      lcd_line(lcd, "Состояние работы", 2);
      lcd_line(lcd, tmp_time, 3);

      break;
    case 2000:
      time(&rawtime);
      timeinfo = localtime(&rawtime);
      //sprintf(tmp_value,"%2d%02d%2d %2d:%02d:%02d", timeinfo->tm_mday,1+timeinfo->tm_mon,1900+timeinfo->tm_year,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
      sprintf(tmp_time, "%2d/%02d  %2d:%02d:%02d", timeinfo->tm_mday,
          1 + timeinfo->tm_mon, timeinfo->tm_hour, timeinfo->tm_min,
          timeinfo->tm_sec);
      //0xEF градус
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
      break;
    case 10:
      //mem_load = (int)(((double)mem_used()/(double)mem_tot)*100);
      lcd_line(lcd, "Меню настроек", 0);
      lcd_line(lcd, "Улица          <", 1);
      lcd_line(lcd, "Сайт", 2);
      lcd_line(lcd, "Миксер", 3);
      break;
    case 20:
      //mem_load = (int)(((double)mem_used()/(double)mem_tot)*100);
      lcd_line(lcd, "Меню настроек", 0);
      lcd_line(lcd, "Улица           ", 1);
      lcd_line(lcd, "Сайт           <", 2);
      lcd_line(lcd, "Миксер", 3);
      break;
    case 30:
      //mem_load = (int)(((double)mem_used()/(double)mem_tot)*100);
      lcd_line(lcd, "Меню настроек", 0);
      lcd_line(lcd, "Улица           ", 1);
      lcd_line(lcd, "Сайт            ", 2);
      lcd_line(lcd, "Миксер         <", 3);
      break;
    case 40:
      //mem_load = (int)(((double)mem_used()/(double)mem_tot)*100);
      lcd_line(lcd, "Меню настроек", 0);
      lcd_line(lcd, "Сайт            ", 1);
      lcd_line(lcd, "Миксер          ", 2);
      lcd_line(lcd, "Тен            <", 3);
      break;

    case 11:
      //mem_load = (int)(((double)mem_used()/(double)mem_tot)*100);
      lcd_line(lcd, "Меню Улица", 0);
      lcd_line(lcd, "Температура    <", 1);
      lcd_line(lcd, "Дельта          ", 2);
      lcd_line(lcd, "Ещё что то      ", 3);
      break;

    case 12:
      //mem_load = (int)(((double)mem_used()/(double)mem_tot)*100);
      lcd_line(lcd, "Меню Улица", 0);
      lcd_line(lcd, "Температура     ", 1);
      lcd_line(lcd, "Дельта         <", 2);
      lcd_line(lcd, "Ещё что то      ", 3);
      break;

    case 13:
      //mem_load = (int)(((double)mem_used()/(double)mem_tot)*100);
      lcd_line(lcd, "Меню Улица", 0);
      lcd_line(lcd, "Температура     ", 1);
      lcd_line(lcd, "Дельта          ", 2);
      lcd_line(lcd, "Ещё что то     <", 3);
      break;

    case 110:
      //mem_load = (int)(((double)mem_used()/(double)mem_tot)*100);
      //printf("%d.00\n",tek_znach);
      sprintf(buffer, "    %d.00  ", tek_znach);
      lcd_line(lcd, "Меню Температура", 0);
      lcd_line(lcd, "                ", 1);
      lcd_line(lcd, buffer, 2);
      lcd_line("                ", 3);
      break;

    case mode_NET:
      lcd_line(lcd, " IP ADDRESS", 0);
      //sprintf(tmp_value,"%s", net_address());
      lcd_line(lcd, tmp_value, 1);
      lcd_line(lcd, "      ", 2);
      lcd_line(lcd, "[CPU][MEM]>NET<[UPT]", 3);
      break;

    case mode_UPT:
      lcd_line(lcd, "    UP TIME", 0);
      //sprintf(tmp_value,"%d sec",uptime());
      lcd_line(lcd, tmp_value, 1);
      lcd_line(lcd, "     ", 2);
      lcd_line(lcd, "[CPU][MEM][NET]>UPT<", 3);
      break;

    default:
      display_mode = mode_OSN;

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

