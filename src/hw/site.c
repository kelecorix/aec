#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "site.h"
#include "ac.h"
#include "throttle.h"
#include "vent.h"
#include "ow.h"
#include "../config/config.h"

//TODO: Переписать в виде синглетона
//TODO: Добавить мьютексы для многопоточного доступа к переменным

Site* site_new(char* filename) {

  Site* site = malloc(sizeof(Site));
  //initialize hardware
  int i;
  for (i = 0; i < 2; i++) {
    site->acs[i] = ac_new();
    site->vents[i] = vent_new();
  }

  site->th = throttle_new();

  site->penalty = 0;
  site->temp_in_prev = 0;
  site->conn = NULL;

  // Для тестирования на эмуляторе
  site->mount_point = "/mnt/1wire/";
  //site->mount_point = "/bus.0/";

  site->cfg = read_config(filename);

  return site;
}

void site_free() {

  //TODO: очистим ресурсы памяти
}

void run(Site* site) {

  //Cтартуем с режима УВО
  site_mode_uvo(site);

}

/* Режим охлаждения УВО */
int site_mode_uvo(Site* site) {
  site->mode = 1;
  site->time_pre = time(NULL);

  int a, v, ret;
  char *key = "temp_dew"; // температура росы
  double temp_dew = atof(getStr(site->cfg, (void *) key));

  // читаем датчики
  ret = read_sensors(site);
  if (ret != 0) {
    //Ошибка чтения датчиков
    site_mode_fail_uvo(site);
  }

  if ((site->temp_out) > temp_dew) {
    //да
    site->th->set_position(site->th, 255);
    site->th->time_start = time(NULL);
  } else {
    //нет
    site->th->set_position(site->th, 0);
    site->th->time_start = time(NULL);
  }

  while (1) {

    // читаем датчики
    ret = read_sensors(site);
    if (ret != 0) {
      //Ошибка чтения датчиков
      site_mode_fail_uvo(site);
    }

    if (difftime(time(NULL), site->time_pre) >= 30) { //секунды

      // start using ventilation
      if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
        // yes

      } else {
        //no
        char *key = "temp_fail";
        double temp_fail = atof(getStr(site->cfg, (void *) key));

        if ((site->temp_in) >= temp_fail) {

          // прерываем цикл
          // переходим в аварийный режим охлаждения
          site_mode_fail_gen(site);

        } else {

          // температура нормальная
          char *key = "temp_support";
          double temp_support = atof(getStr(site->cfg, (void *) key));

          if (site->temp_in >= (temp_support - 2)
              && site->temp_in <= (temp_support) + 2) {
            // да

            if ((temp_support - (site->temp_out)) >= 2) {
              //да

              //выключим вентиляцию
              for (v = 0; v < 2; v++) {
                if (site->vents[v]->mode == 1)
                  site->vents[v] = 0;
              }

            } else {

              //обнуляем данные TODO: проставить время окончания, высчитать время работы, записать в лог, затем обнулить
              site->time_uvo = 0; //TODO: проверить присвоение
              site->th->time_start = 0;
              site->vents[0]->time_start = 0;
              site->vents[1]->time_start = 0;

              if (site->temp_out <= site->temp_in) {
                site->vents[0]->mode = 1;
                site->vents[1]->mode = 2;
                set_turns(site->vents[0], 255);
                set_turns(site->vents[1], 255);

              }

              if (site->power == 0) {
                //питания нет
                site->acs[0]->error = NOPOWER;
                site->acs[1]->error = NOPOWER;

                // переходим в режим охлаждения УВО
                // авария на кондиционере
                site_mode_fail_ac(site);

              } else {
                //питание есть
                // переходим в режим охлаждения кондиционером
                site_mode_ac(site);

              }

            }

          } else {
            //нет
            //обнуляем данные TODO: проставить время окончания, высчитать время работы, записать в лог, затем обнулить
            site->time_uvo = 0; //TODO: проверить присвоение
            site->th->time_start = 0;
            site->vents[0]->time_start = 0;
            site->vents[1]->time_start = 0;

            if (site->temp_in < temp_support - 2) {
              //да

              // выключим вентиляцию
              if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
                site->vents[0]->mode = 0;
                site->vents[1]->mode = 0;
                site->vents[0]->turns = 0;
                site->vents[1]->turns = 0;
              }

              // выключим кондиционеры
              if (site->acs[0]->mode == 1 || site->acs[1]->mode == 1) {
                site->acs[0]->mode = 0;
                site->acs[1]->mode = 0;
              }

              char *key = "temp_heat";
              double temp_heat = atof(getStr(site->cfg, (void *) key));

              if ((site->temp_in) < temp_heat) {
                // переходим в режим обогрева
                site_mode_heat(site);
              } else {
                // продолжаем работу в режиме уво
                continue;
              }

            } else {
              //нет
              if (site->power == 0) {
                //питания нет
                site->acs[0]->error = NOPOWER;
                site->acs[1]->error = NOPOWER;

                // переходим в режим охлаждения УВО
                // авария на кондиционере
                site_mode_fail_ac(site);

              } else {
                //питание есть
                site->penalty++;
                // переходим в режим охлаждения кондиционером
                site_mode_ac(site);

              }

            }

          }

        }

      }

    } else {
      continue;
    }

  }

  return 1;

}

/* Режим охлаждения кондиционером */
int site_mode_ac(Site* site) {

  site->mode = 2;
  site->time_pre = time(NULL);

  int a, v;
  for (v = 0; v < 2; v++) {
    site->vents[v]->time_start = 0;
  }

  for (a = 0; a < 2; a++) {
    site->acs[a]->is_diff = 0;
  }

  char *key = "num_ac";
  int num_ac = atoi(getStr(site->cfg, (void *) key));

  int num_ac_tmp = num_ac;

  if (site->th->position == 255) {

    site->th->position = 0;
    site->th->time_start = time(NULL);

  }

  for (a = 0; a < 2; a++) {
    site->acs[a]->mode = 1;
    site->acs[a]->time_start = time(NULL);
  }

  while (1) { // sensors was read - ok

    int ret;
    ret = read_sensors(site);
    if (ret != 0) {
      //Ошибка чтения датчиков
      site_mode_fail_ac(site);
    }

    if (difftime(time(NULL), site->time_pre) <= 30) { //секунды
      continue;
    } else {

      site->time_pre = time(NULL);

      for (a = 0; a < 2; a++) {
        if (((site->temp_in - site->acs[a]->temp) >= 10)
            && (site->acs[a]->mode == 1)) {
          //да
          if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
            // да
            // выключим вентиляцию
            for (v = 0; v < 2; v++) {
              site->vents[v]->mode = 0;
              site->vents[v]->mode = 0;
            }

            site->acs[a]->mode = 1;
            site->acs[1]->time_start = time(NULL);

          }

        } else {
          //нет
          site->acs[a]->mode = 0;

        }

        //600
        // отработан промежуток?
        if ((difftime(time(NULL), site->acs[a]->time_start) > 600)
            && (site->acs[a]->mode == 1)) {
          //дельта набрана?
          if (site->acs[a]->is_diff == 0) {
            // Авария кондиционера
            num_ac_tmp--;
            site->acs[a]->mode = 0;
          }

        }

      }

      // еще есть живые кондиционеры?
      if (num_ac_tmp > 0) {
        //да
        char *key = "temp_support";
        double temp_support = atof(getStr(site->cfg, (void *) key));

        *key = "temp_fail";
        double temp_fail = atof(getStr(site->cfg, (void *) key));

        if ((temp_support - site->temp_out) >= (site->penalty + 2)) {
          // переходим в УВО
          site_mode_uvo(site);

        } else {
          if (site->temp_in >= temp_fail) {
            //переходим в аварийный режим
            site_mode_fail_ac(site);
          } else {
            // работа нормальная
            continue;
          }

        }

      } else {
        // авария кондиционирования
        // переходим на УВО
        site_mode_uvo(site);
      }

    }

  }

  return 1;
}

/* Режим догрева сайта */
int site_mode_heat(Site* site) {

  site->mode = 3;

  int a, v;

  site->time_pre = time(NULL);
  site->th_check = 0;
  site->vents[0]->time_start = 0;
  site->vents[1]->time_start = 0;
  site->vents[0]->error = ERROR_HEAT;
  site->vents[1]->error = ERROR_HEAT;

  if (site->th->position == 255) {
    // улица
    site->th->set_position(site->th, 0);
    site->th->time_start = time(NULL);
  } else {
    //сайт

    while (1) { // sensors was read - ok

      int th_pos_read;

      // читаем датчики
      int ret;
      ret = read_sensors(site);
      if (ret != 0) {
        //Ошибка чтения датчиков
        site_mode_fail_uvo(site);
      }

      if (difftime(time(NULL), site->time_pre) <= 30) { //секунды
        continue;
      } else {

        if ((difftime(time(NULL), site->th->time_start) >= 30)
            && site->th_check == 0) {
          //да
          // не было проверки закслонки
          if (site->th->position == th_pos_read) {
            //да
            site->th_check = 1;

            site->time_pre = time(NULL);

            char *key = "temp_support";
            double temp_support = atof(getStr(site->cfg, (void *) key));

            if (site->temp_in >= temp_support) {
              //да
              site->ten = 0;

              if (site->temp_mix > 60) {
                continue; // продолжаем цикл догрева
              } else {

                site->vents[0]->turns = 0; // обороты приточного

                //выключим вентиляцию
                for (v = 0; v < 2; v++) {
                  if (site->vents[v]->mode == 1)
                    site->vents[v] = 0;
                }

                site_mode_uvo(site); // режим охлаждения уво

              }

            } else {
              //нет

              if (site->vents[0]->mode == 0 || site->vents[1]->mode == 0) {
                site->vents[0] = 255;
                site->vents[0]->time_start = time(NULL);
                site->vents[0] = 1;

              }

              if ((difftime(time(NULL), site->vents[0]) > 30)
                  || (difftime(time(NULL), site->vents[0]) > 30)) {

                //да

                if (site->th->position == th_pos_read) {
                  //да
                  site->vents[0]->time_start = time(NULL);
                  if ((site->ten == 1) && site->vents[0]->error == ERROR_HEAT) {

                    //да
                    continue;

                  } else {
                    //нет
                    site->ten = 1;
                    continue;

                  }

                } else {
                  //нет
                  site->ten = 0;
                  site->vents[0]->turns = 0;
                  site->vents[0]->error = ERROR;

                  site_mode_ac(site); // Авария вентиляции - охлаждение кондиционером

                }

              } else {

                //нет
                if (((site->ten == 1) && site->vents[0]->error == ERROR_HEAT)
                    || ((site->ten == 1) && site->vents[1]->error == ERROR_HEAT)) {

                  //да
                  continue;

                } else {
                  //нет
                  site->ten = 1;
                  continue;

                }

              }

            }

          } else {
            //нет
            site->ten = 0;
            if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
              //да

              if (site->temp_mix > 60) {
                continue; // продолжаем цикл догрева
              } else {

                site->vents[0]->turns = 0; // обороты приточного

                //выключим вентиляцию
                for (v = 0; v < 2; v++) {
                  if (site->vents[v]->mode == 1)
                    site->vents[v] = 0;
                }

                site_mode_ac(site); // Авария заслонки- охлаждение кондиционером

              }

            } else {
              //нет
              site_mode_ac(site); // Авария заслонки- охлаждение кондиционером

            }

          }

        } else {
          //нет

        }

      }

    }

  }

  return 1;
}

/* Авария УВО -
 * Авария вентиляторов - Авария датчиков -
 * Авария заслонки - Авария охлаждения кондиционером */
int site_mode_fail_uvo(Site* site) {
  site->mode = 4;
  site->time_pre = time(NULL);

  int a, v;
  for (v = 0; v < 2; v++) {
    site->vents[v]->time_start = 0;
  }

  for (a = 0; a < 2; a++) {
    site->acs[a]->is_diff = 0;
  }

  char *key = "num_ac";
  int num_ac = atoi(getStr(site->cfg, (void *) key));

  int num_ac_tmp = num_ac;

  while (1) { // sensors was read - ok

    int ret;
    ret = read_sensors(site);
    if (ret != 0) {
      //Ошибка чтения датчиков
      continue;
    }

    if (difftime(time(NULL), site->time_pre) <= 30) { //секунды
      continue;
    } else {

      site->time_pre = time(NULL);

      for (a = 0; a < 2; a++) {
        if ((site->temp_in >= 10) && (site->acs[a]->mode == 1)) {
          //да
          if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
            //да
            // выключим вентиляцию
            for (v = 0; v < 2; v++) {
              site->vents[v]->mode = 0;
              site->vents[v]->mode = 0;
            }

            site->acs[a]->mode = 1;
            site->acs[1]->time_start = time(NULL);

          }

        }

        //600
        // отработан промежуток?
        if ((difftime(time(NULL), site->acs[a]->time_start) > 600)
            && (site->acs[a]->mode == 1)) {
          //дельта набрана?
          if (site->acs[a]->is_diff == 0) {
            // Авария кондиционера
            num_ac_tmp--;
            site->acs[a]->mode = 0;
          }

        }

      }

      // еще есть живые кондиционеры?
      if (num_ac_tmp > 0) {
        //да
        char *key = "temp_support";
        double temp_support = atof(getStr(site->cfg, (void *) key));

        if ((temp_support - site->temp_out) >= (site->penalty + 2)) {
          if (site->temp_in <= temp_support - 2) {
            // переходим в УВО
            site_mode_uvo(site);
          } else {
            //продолжаем в текущем режиме
            continue;
          }
        } else {
          //продолжаем в текущем режиме
          continue;
        }

      } else {
        // авария кондиционирования
        // переходим на УВО
        site_mode_uvo(site);
      }

    }

  }

  return 1;
}

/* Авария кондиционеров - Охлаждение УВО */
int site_mode_fail_ac(Site* site) {

  site->mode = 5;

  site->time_pre = time(NULL);
  site->th_check = 0;
  site->th->time_start = 0;

  int a, v;

  if (site->th->position != 255) {
    site->th->position = 255;
    site->th->time_start = time(NULL);

  }

  while (1) {

    int th_pos_read;
    int vents_r[2];

    int ret;
    ret = read_sensors(site);
    if (ret != 0) {
      //Ошибка чтения датчиков
      for (v = 0; v < 2; v++) {
        site->vents[v]->mode = 1;
        site->vents[v]->turns = 8; // 100%
        site->vents[v]->time_start = time(NULL);
      }

      continue;

    }

    if (difftime(time(NULL), site->time_pre) <= 30) { //секунды
      continue;
    } else {
      if ((difftime(time(NULL), site->th->time_start) >= 30)
          && site->th_check == 0)

        if (site->th->position != th_pos_read) {
          for (v = 0; v < 2; v++) {
            site->vents[v]->mode = 1;
            site->vents[v]->turns = 8; // 100%
            site->vents[v]->time_start = time(NULL);
            site_mode_uvo(site); // перейдем в режим УВО
          }

        } else {

          site->th_check = 1;
          continue;

        }

    }

    site->time_pre = time(NULL);

    // выключим все кондиционеры
    for (a = 0; a < 2; a++) {
      site->acs[v]->mode = 0;
    }

    // включим вентиляцию
    for (v = 0; v < 2; v++) {
      if (site->vents[v]->mode == 0) {
        site->vents[v]->turns = 8; // 100%
        site->vents[v]->time_start = time(NULL);
      }

    }

    //работаем с вытяжным
    if ((difftime(time(NULL), site->vents[0]->time_start) > 30)) {
      if (site->vents[0]->turns == vents_r[0]) {
        site->vents[0]->error = NOERROR;
      } else {
        // Заглушка, можем уйти в бесконечный цикл
        // site_mode_ac(site);
      }

    }

    // проверяем кондиционеры
    //
    for (a = 0; a < 2; a++) {
      if (site->acs[a]->error == NOPOWER) {
        if (site->power == 1)
          site_mode_ac(site); // переходим в нормальный режим;
      }
    }

    char *key = "temp_support";
    double temp_support = atof(getStr(site->cfg, (void *) key));

    if (site->temp_in > temp_support - 2) {
      //Переходим на уво
      site_mode_uvo(site);

    }

    // выключим вениляцию
    for (v = 0; v < 2; v++) {
      if (site->vents[v]->mode == 1) {
        site->vents[v]->mode = 0;
        site->vents[v]->turns = 0; // 100%
        site->vents[v]->time_stop = time(NULL);
      }
    }

    //выключим кондиционеры
    for (a = 0; a < 2; a++) {
      site->acs[a]->mode = 0;
      site->acs[a]->time_stop = time(NULL);
    }

    *key = "temp_heat";
    double temp_heat = atof(getStr(site->cfg, (void *) key));

    if (site->temp_in < temp_heat) {
      site_mode_heat(site);
    } else {
      continue;
    }

  }
  return 1;
}

/* Превышена температура аварии - Аварийный режим охлаждения*/
int site_mode_fail_gen(Site* site) {
  site->mode = 6;
  return 1;
}

ConfigTable* read_config(char* filepath) {
  ConfigTable* cfg = readConfig(filepath);
  return cfg;
}

int set_mode(Site* site, int val) {

  return 0;

  if (val >= 0 && val <= 6) { // acceptable numbers
    site->mode = val;
    if (val == 1)
      site_mode_uvo(site);
    if (val == 2)
      site_mode_ac(site);
    if (val == 3)
      site_mode_heat(site);
    if (val == 4)
      site_mode_fail_uvo(site);
    if (val == 5)
      site_mode_fail_ac(site);
    if (val == 6)
      site_mode_fail_gen(site);
  }

  return 0;
}

