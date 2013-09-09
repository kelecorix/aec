#include <stdio.h>
#include <time.h>

#include "ui.h"
#include "lcd.h"
#include "../hw/site.h"

void run_ui(Site* site) {

  //прочитаем адрес из конфигурации
  char *key = "a_lcd";
  char *a_lcd = getStr(site->cfg, (void *) key);
  int addr = atoi(a_lcd);

  LCD* lcd = lcd_new(addr);

  int menu_curr = 0; // текущее меню
  int val_curr = 0;  // текущее значение
  int out_curr = 40; // текущий вывод, смеситель и кондиционер попеременно

  //int display_mode = mode_intro;
  display(lcd, 1000);

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

    //display_mode = keyboard();
    menu_curr = keyboard(menu_curr);
    //printf("%d\n",tek_menu);
    out_curr--;
    usleep(10000);
  }
}

void display(Site* site, LCD* lcd, int display_mode) {

  char tmp_value[20], tmp_time[20], tmp_temp_ulica[20],
       tmp_temp_sayt[20], tmp_temp_miks[20], tmp_temp_kond[20],
       buffer[200];
  char* host;
  int mem_tot, i_cpu_load, cpu_interval = 0, tek_znach = 25;
  long double b[7];
  time_t rawtime;
  struct tm * timeinfo;
  //-----
  char *key;

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
      sprintf(tmp_temp_ulica, "Улица  = %2.2f°C", site->temp_out);
      //lcd_line("Улица  = 20.00°C",0);
      lcd_line(lcd, tmp_temp_ulica, 0);
      lcd_line(lcd, "Сайт   = 18.00°C", 1);
      lcd_line(lcd, "Миксер = 18.00°C", 2);
      lcd_line(lcd, tmp_time, 3);
      //lcd_line(">CPU<[MEM][NET][UPT]",2);
      break;
    case 2000:
      time(&rawtime);
      timeinfo = localtime(&rawtime);
      //sprintf(tmp_value,"%2d%02d%2d %2d:%02d:%02d", timeinfo->tm_mday,1+timeinfo->tm_mon,1900+timeinfo->tm_year,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
      sprintf(tmp_time, "%2d/%02d  %2d:%02d:%02d", timeinfo->tm_mday,
          1 + timeinfo->tm_mon, timeinfo->tm_hour, timeinfo->tm_min,
          timeinfo->tm_sec);
      //0xEF градус
      lcd_line(lcd, "Улица  = 20.00°C", 0);
      lcd_line(lcd, "Сайт   = 18.00°C", 1);
      lcd_line(lcd, "Конд   =  8.00°C", 2);
      lcd_line(lcd, tmp_time, 3);
      //lcd_line(">CPU<[MEM][NET][UPT]",2);
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

