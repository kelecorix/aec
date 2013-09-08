#include <stdio.h>
#include <time.h>

#include "ui.h"
#include "lcd.h"
#include "../hw/site.h"

char tmp_value[20];
char tmp_time[20];
char tmp_temp_ulica[20];
char tmp_temp_sayt[20];
char tmp_temp_miks[20];
char tmp_temp_kond[20];
int mem_tot;
long double b[7];
int cpu_interval = 0;
int i_cpu_load;
char* host;
time_t rawtime;
struct tm * timeinfo;
char buffer[200];
int tek_znach = 25;

void run_ui(Site* site) {

  // установим значение адреса экрана

  // установим значение адреса клавиатуры


  int tek_menu = 0;
  int tek_znach = 0;
  int tek_vyvod = 40; //будем выводить то миксер то кондишку
  //int display_mode = mode_intro;
  display(1000);

  //usleep(5000000);
  usleep(500000);

  //display_mode = mode_OSN;

  for (;;) {

    /***********************************************************
     * Выводим по переменно температуру миксера или кондиционера
     * tek_menu == 0    - миксер
     * tek_menu == 2000 - кондиционер
     */
    if (tek_vyvod == 0) {
      if (tek_menu == 0 || tek_menu == 2000) {
        if (tek_menu == 0) {
          tek_menu = 2000;
        } else {
          tek_menu = 0;
        }

      }
      tek_vyvod = 40;
    }
    /**********************************************************
     *
     */

    display(tek_menu);

    //display_mode = keyboard();
    tek_menu = keyboard(tek_menu);
    //printf("%d\n",tek_menu);
    tek_vyvod--;
    usleep(10000);

  }

}

void display(int display_mode) {

  switch (display_mode) {
    case 1000:
      lcd_line("     Меандр", 0);       // TODO: из конфига
      lcd_line("FreCooling v.0.1", 1);  // TODO: из конфига
      lcd_line("Raspberri PI I2C", 2);  // TODO: из конфига
      lcd_line("rev.h 0.1", 3);         // TODO: из конфига
      break;
    case 0:
      time(&rawtime);
      timeinfo = localtime(&rawtime);
      //sprintf(tmp_value,"%2d%02d%2d %2d:%02d:%02d", timeinfo->tm_mday,1+timeinfo->tm_mon,1900+timeinfo->tm_year,timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
      sprintf(tmp_time, "%2d/%02d  %2d:%02d:%02d", timeinfo->tm_mday,
          1 + timeinfo->tm_mon, timeinfo->tm_hour, timeinfo->tm_min,
          timeinfo->tm_sec);
      //0xEF градус
      sprintf(tmp_temp_ulica, "Улица  = %2.2f°C", 25.22);
      //lcd_line("Улица  = 20.00°C",0);
      lcd_line(tmp_temp_ulica, 0);
      lcd_line("Сайт   = 18.00°C", 1);
      lcd_line("Миксер = 18.00°C", 2);
      lcd_line(tmp_time, 3);
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
      lcd_line("Улица  = 20.00°C", 0);
      lcd_line("Сайт   = 18.00°C", 1);
      lcd_line("Конд   =  8.00°C", 2);
      lcd_line(tmp_time, 3);
      //lcd_line(">CPU<[MEM][NET][UPT]",2);
      break;
    case 10:
      //mem_load = (int)(((double)mem_used()/(double)mem_tot)*100);
      lcd_line("Меню настроек", 0);
      lcd_line("Улица          <", 1);
      lcd_line("Сайт", 2);
      lcd_line("Миксер", 3);
      break;
    case 20:
      //mem_load = (int)(((double)mem_used()/(double)mem_tot)*100);
      lcd_line("Меню настроек", 0);
      lcd_line("Улица           ", 1);
      lcd_line("Сайт           <", 2);
      lcd_line("Миксер", 3);
      break;
    case 30:
      //mem_load = (int)(((double)mem_used()/(double)mem_tot)*100);
      lcd_line("Меню настроек", 0);
      lcd_line("Улица           ", 1);
      lcd_line("Сайт            ", 2);
      lcd_line("Миксер         <", 3);
      break;
    case 40:
      //mem_load = (int)(((double)mem_used()/(double)mem_tot)*100);
      lcd_line("Меню настроек", 0);
      lcd_line("Сайт            ", 1);
      lcd_line("Миксер          ", 2);
      lcd_line("Тен            <", 3);
      break;

    case 11:
      //mem_load = (int)(((double)mem_used()/(double)mem_tot)*100);
      lcd_line("Меню Улица", 0);
      lcd_line("Температура    <", 1);
      lcd_line("Дельта          ", 2);
      lcd_line("Ещё что то      ", 3);
      break;

    case 12:
      //mem_load = (int)(((double)mem_used()/(double)mem_tot)*100);
      lcd_line("Меню Улица", 0);
      lcd_line("Температура     ", 1);
      lcd_line("Дельта         <", 2);
      lcd_line("Ещё что то      ", 3);
      break;

    case 13:
      //mem_load = (int)(((double)mem_used()/(double)mem_tot)*100);
      lcd_line("Меню Улица", 0);
      lcd_line("Температура     ", 1);
      lcd_line("Дельта          ", 2);
      lcd_line("Ещё что то     <", 3);
      break;

    case 110:
      //mem_load = (int)(((double)mem_used()/(double)mem_tot)*100);
      //printf("%d.00\n",tek_znach);
      sprintf(buffer, "    %d.00  ", tek_znach);
      lcd_line("Меню Температура", 0);
      lcd_line("                ", 1);
      lcd_line(buffer, 2);
      lcd_line("                ", 3);
      break;

    case mode_NET:
      lcd_line(" IP ADDRESS", 0);
      //sprintf(tmp_value,"%s", net_address());
      lcd_line(tmp_value, 1);
      lcd_line("      ", 2);
      lcd_line("[CPU][MEM]>NET<[UPT]", 3);
      break;

    case mode_UPT:
      lcd_line("    UP TIME", 0);
      //sprintf(tmp_value,"%d sec",uptime());
      lcd_line(tmp_value, 1);
      lcd_line("     ", 2);
      lcd_line("[CPU][MEM][NET]>UPT<", 3);
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

