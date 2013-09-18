#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "site.h"
#include "ac.h"
#include "throttle.h"
#include "vent.h"
#include "ow.h"
#include "i2c.h"
#include "../config/config.h"
#include "../log/logger.h"
//Site* site;

void site_free() {
  //TODO: очистим ресурсы памяти
}

void run(Site* site) {

  //write_log(site->logger->eventLOG, "Начало работы");

  // Установим значение регистра реле в 0
  i2cOpen();
  int addr = strtol(getStr(site->cfg, "a_relay"), NULL, 16);
  write_log(site->logger->eventLOG,
      ssprintf("Управляем регистром, адрес %x\n", addr));
  set_i2c_register(g_i2cFile, addr, 0xFF, 0xFF);
  i2cClose();

  // По умолчанию
  site->th->set_position(site->th, 0);
  site->th_r_exists = 0;
  site->tacho1_exists = 1;
  site->tacho2_exists = 1;

  site_mode_uvo(site);
  //site_mode_fail_uvo(site);
  //site_mode_fail_ac(site);
}

/* Режим охлаждения УВО */
int site_mode_uvo(Site* site) {
  write_log(site->logger->eventLOG, ssprintf("Режим охлаждения УВО!\n"));
  //write_log(site->logger->eventLOG, "Режим охлаждения УВО");
  site->mode = 1;
  site->time_pre = time(NULL);
  site->time_uvo = time(NULL);

  int a, ret, res;
  float temp_dew = strtof(getStr(site->cfg, (void *) "temp_dew"), NULL);

  //По умолчанию: кондиц. вкл.
  for (a = 0; a < 2; a++) {
    site->acs[a]->set_mode(site->acs[a], 1);
  }

  //остановим вентиляторы
  site->vents[0]->set_step(site->vents[0], 0);
  site->vents[1]->set_step(site->vents[1], 0);

  // читаем датчики
  ret = read_sensors(site);
  if (ret != 0) {
    //Ошибка чтения датчиков
    site_mode_fail_uvo(site);
  }

  write_log(site->logger->eventLOG,
      ssprintf("Переведем заслонку site->temp_out = %f temp_dew = %f\n",
          site->temp_out, temp_dew));
  if ((site->temp_out) > temp_dew) {
    if (site->th->exist) //Это есть ли заслонка? или есть ли откуда читать
    {
      write_log(site->logger->eventLOG,
          ssprintf("переведем заслонку в положение улица\n"));
      site->th->set_position(site->th, 10); //переведем заслонку в положение улица
      site->th->time_start = time(NULL);
    }
  } else {
    if (site->th->exist) {
      write_log(site->logger->eventLOG,
          ssprintf("переведем заслонку в положение сайт\n"));
      site->th->set_position(site->th, 0); //переведем заслонку в положение сайт
      site->th->time_start = time(NULL);
    }
  }

  write_log(site->logger->eventLOG, ssprintf("Перед while UVO"));
  while (1) {
    ret = read_sensors(site);
    if (ret != 0) {
      site_mode_fail_uvo(site);
    }

    site->power = 1;

    if (difftime(time(NULL), site->time_pre) <= 5) {
      continue;
    }

    write_log(site->logger->eventLOG,
        ssprintf(
            "****************Принятие решения Режим УВО*****************\n"));
    site->time_pre = time(NULL);
    write_log(site->logger->eventLOG,
        ssprintf("site->vents[0]->mode = %d site->vents[1]->mode = %d\n",
            site->vents[0]->mode, site->vents[1]->mode));
    if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
      write_log(site->logger->eventLOG,
          ssprintf("Вентиляторы включены принятие решения\n"));
      write_log(site->logger->eventLOG,
          ssprintf("Время разница: %d\n", (time(NULL) - site->time_uvo)));
      float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"),
      NULL);
      if (((site->vents[0]->step == 10 || site->vents[1]->step == 10)
          && ((time(NULL) - site->time_uvo) >= 60))
          || (site->temp_in <= temp_support)) {
        write_log(site->logger->eventLOG,
            ssprintf(
                "Вентиляторы вращаются на максимум и проработали 300 сек\n"));
        res = sub_uvo_fail(site); // 1 - EXIT_FAILURE - не попали в ошибку
        if (res)
          sub_uvo_vent(site);
        else
          continue;
      } else {
        write_log(site->logger->eventLOG,
            ssprintf("вентиляторы не на максимуме или не прошло 300 сек\n"));
        sub_uvo_vent(site);
      }
    } else {
      write_log(site->logger->eventLOG, ssprintf("Вентиляторы не включены\n"));

      res = sub_uvo_fail(site); // 1 - EXITE_FAILURE - не попали в ошибку
      if (res)
        sub_uvo_vent(site);
      else
        continue;
    }
  }

  return 1;
}

void sub_uvo_vent(Site* site) {

  int a, v;
  float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"), NULL);
  float temp_heat = strtof(getStr(site->cfg, (void *) "temp_heat"), NULL);

  write_log(site->logger->eventLOG,
      ssprintf("Среда позваляет работать на вентиляторах?\n"));
  if ((temp_support - site->temp_out) >= 2) {
    write_log(site->logger->eventLOG, ssprintf("Да позваляет\n"));
    if ((site->acs[0]->mode == 1) || (site->acs[1]->mode == 1)) {
      write_log(site->logger->eventLOG,
          ssprintf("Кондиционеры были включены выключим\n"));
      for (a = 0; a < 2; a++) {
        site->acs[a]->set_mode(site->acs[a], 0);
      }
    }

    if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
      write_log(site->logger->eventLOG,
          ssprintf("Вентиляторы включены site->vents[0]->time_start = %d \n",
              site->vents[0]->time_start));
      if ((time(NULL) - site->vents[0]->time_start) > 30) {
        if (site->tacho1_exists && site->tacho2_exists) {
          write_log(site->logger->eventLOG,
              ssprintf(
                  "Проверим тахо site->vents[0]->step = %d site->tacho1 = %d, обороты %d | site->vents[1]->step = %d site->tacho2 = %d обороты %d \n",
                  site->vents[0]->step, site->tacho1, site->tacho1_t,
                  site->vents[1]->step, site->tacho2, site->tacho2_t));
          //if ((site->vents[0]->step != site->tacho1)
          //    || (site->vents[1]->step != site->tacho2)) {
          if ((site->tacho1 < 6) || (site->tacho2 < 6)) {
            write_log(site->logger->eventLOG,
                ssprintf("Какой-то вентилятор не вращается\n"));
            for (v = 0; v < 2; v++) {
              site->vents[v]->error = ERROR;
            }
            write_log(site->logger->eventLOG,
                ssprintf(
                    "Авария уво переход на аварийное охлаждение кондиционером\n"));
            site_mode_fail_uvo(site);
          } else {
            write_log(site->logger->eventLOG,
                ssprintf("Вентиляторы вращаются\n"));
            for (v = 0; v < 2; v++) {
              site->vents[v]->error = NOERROR;
            }
          }
        }
      }

      if ((time(NULL) - site->time_uvo) >= 1800) {
        write_log(site->logger->eventLOG, ssprintf("Сняли пенальти с уво\n"));
        site->penalty = 0;
      }

      int curr_step = site->vents[0]->step;
      write_log(site->logger->eventLOG,
          ssprintf(
              "curr_step = %d site->temp_in_prev = %f site->temp_in = %f\n",
              curr_step, site->temp_in_prev, site->temp_in));

      if (site->temp_in_prev != site->temp_in_prev_prev != site->temp_in) {
        if (site->temp_in_prev <= site->temp_in) {
          write_log(site->logger->eventLOG,
              ssprintf("Добавим обороты curr_step = %d\n", curr_step));
          if (curr_step != 10) {
            curr_step++;
            if (curr_step > 10) {
              curr_step = 10;
            }
            for (v = 0; v < 2; v++) {
              site->vents[v]->set_step(site->vents[v], curr_step);
              site->vents[v]->time_start = time(NULL);
            }
          }
        } else {
          write_log(site->logger->eventLOG,
              ssprintf("Уменьшим обороты curr_step = %d\n", curr_step));
          if (curr_step != 0) {
            curr_step--;
            if (curr_step < 0) {
              curr_step = 0;
            }
            for (v = 0; v < 2; v++) {
              site->vents[v]->set_step(site->vents[v], curr_step);
              site->vents[v]->time_start = time(NULL);
            }
          }
        }
      }
      site->temp_in_prev = site->temp_in;
      write_log(site->logger->eventLOG, ssprintf("sub_uvo_th\n"));
      sub_uvo_th(site, 0);
    } else {
      write_log(site->logger->eventLOG,
          ssprintf("sub_uvo_vent Вентиляторы не включены включим\n"));
      site->time_uvo = time(NULL);
      //site->temp_in_prev = site->temp_in; //видимо не сдесь так как тут мы только когда включаем

      if ((site->temp_out > 20)
          && (site->vents[0]->error == ERROR || site->vents[1]->error == ERROR)) {
        for (v = 0; v < 2; v++) {
          site->vents[v]->set_step(site->vents[v], 10);
          site->vents[v]->time_start = time(NULL);
        }
      } else {
        for (v = 0; v < 2; v++) {
          site->vents[v]->set_step(site->vents[v], 2);
          site->vents[v]->time_start = time(NULL);
        }
      }

      for (v = 0; v < 2; v++) {
        site->vents[v]->time_start = time(NULL);
      }

      sub_uvo_th(site, 0);
    }
  } else {
    write_log(site->logger->eventLOG,
        ssprintf("Температура не позваляет работать на вентиляторах\n"));
    sub_uvo_pow(site);
  }
}

int sub_uvo_pen(Site* site) {

  write_log(site->logger->eventLOG, ssprintf("Мы в sub_uvo_pen\n"));
  int a, v;
  float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"), NULL);
  float temp_heat = strtof(getStr(site->cfg, (void *) "temp_heat"), NULL);

  site->time_uvo = 0;
  site->th->time_start = 0;
  for (v = 0; v < 2; v++) {
    site->vents[v]->time_start = 0;
  }

  if (site->temp_in < (temp_support - 2)) {
    write_log(site->logger->eventLOG,
        ssprintf("Температура ниже поддержания - 2\n"));
    if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
      write_log(site->logger->eventLOG, ssprintf("Выключим вентиляторы\n"));
      for (v = 0; v < 2; v++) {
        site->vents[v]->set_step(site->vents[v], 0);
      }
    }

    if ((temp_support - site->temp_out) <= 2) {
      site_mode_ac(site);
    }

    if (site->acs[0]->mode == 1 || site->acs[1]->mode == 1) {
      write_log(site->logger->eventLOG, ssprintf("Выключим кондиционеры\n"));
      for (a = 0; a < 2; a++) {
        site->acs[a]->set_mode(site->acs[a], 0);
      }
    }

    if (site->temp_in < temp_heat) {
      write_log(site->logger->eventLOG,
          ssprintf("Переходим в режим догрева\n"));
      site_mode_heat(site);
    } else {
      return EXIT_SUCCESS; //вернем 0, чтоб перейти обратно в УВО
    }
  } else {
    write_log(site->logger->eventLOG,
        ssprintf("Температура выше поддержания + 2\n"));
    if (site->power == 0) {
      //питания нет
      for (a = 0; a < 2; a++) {
        site->acs[a]->error = NOPOWER;
      }

      // переходим в режим охлаждения УВО
      // авария на кондиционере
      write_log(site->logger->eventLOG,
          ssprintf(
              "переходим в режим охлаждения УВО  авария на кондиционере\n"));
      site_mode_fail_ac(site);
    } else {
      //питание есть
      site->penalty++;
      // переходим в режим охлаждения кондиционером
      write_log(site->logger->eventLOG,
          ssprintf("переходим в режим охлаждения кондиционером\n"));
      site_mode_ac(site);
    }
  }
  write_log(site->logger->eventLOG, ssprintf("Выход из sub_uvo_pen\n"));
  return EXIT_SUCCESS;
}

void sub_uvo_pow(Site* site) {

  write_log(site->logger->eventLOG, ssprintf("******sub_uvo_pow*******\n"));
  int a, v;

  site->time_uvo = 0;
  site->th->time_start = 0;
  for (v = 0; v < 2; v++) {
    site->vents[v]->time_start = 0;
  }

  if (site->temp_out < site->temp_in) {
    for (v = 0; v < 2; v++) {
      site->vents[v]->set_step(site->vents[v], 255);
    }
  }

  if (site->power == 1) {
    site_mode_ac(site);
  } else {
    for (a = 0; a < 2; a++) {
      site->acs[a]->error = NOPOWER;
    }
    site_mode_fail_ac(site);
  }
}

// fail- флаг работы в аварийном режиме
//
void sub_uvo_th(Site* site, int fail) {

  write_log(site->logger->eventLOG,
      ssprintf("*********sub_uvo_th**************\n"));
  int v;
  float temp_dew = strtof(getStr(site->cfg, (void *) "temp_dew"), NULL);
  write_log(site->logger->eventLOG,
      ssprintf("sub_uvo_th temp_dew = %f site->temp_out = %f \n", temp_dew,
          site->temp_out));
  if (site->temp_out > temp_dew) {
    write_log(site->logger->eventLOG, ssprintf("Где заслонка\n"));
    // Заслонка выставлена на улицу
    if (site->th->position != 10) {
      write_log(site->logger->eventLOG,
          ssprintf("Переводим заслонку на улицу\n"));
      site->th->set_position(site->th, 10);
    }
  } else {
    write_log(site->logger->eventLOG,
        ssprintf(
            "Не настало ли время проверить работает ли заслонка? diff %d\n",
            (time(NULL) - site->th->time_start)));
    if ((time(NULL) - site->th->time_start) >= 120) {
      write_log(site->logger->eventLOG,
          ssprintf("Да настало, а есть ли откуда читать?\n"));
      site->th->time_start = time(NULL);
      if (site->th_r_exists) {
        write_log(site->logger->eventLOG,
            ssprintf(
                "site->th->position = %d site->th_r = %d site->th->position_adc = %d\n",
                site->th->position, site->th_r, site->th->position_adc));
        if (site->th->position == site->th_r) {

          int curr_pos = site->th->position;
          if (site->temp_mix >= temp_dew) {
            site->th->set_position(site->th, curr_pos++);
          } else {
            site->th->set_position(site->th, curr_pos--);
          }
        } else {
          write_log(site->logger->eventLOG, ssprintf("Авария заслонки\n"));
          if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
            for (v = 0; v < 2; v++) {
              site->vents[v]->set_step(site->vents[v], 0);
            }
          }
          if (!fail) {
            site_mode_fail_uvo(site);
          }
        }
      }
    }
    int curr_pos = site->th->position;
    write_log(site->logger->eventLOG,
        ssprintf(
            "Температура в миксере %f температура росы %f site->th->position = %d\n",
            site->temp_mix, temp_dew, site->th->position));
    if (site->temp_mix >= temp_dew) {
      //printf("Приоткроем заслонку %d\n", curr_pos);
      if (curr_pos != 10) {
        curr_pos++;
        site->th->set_position(site->th, curr_pos);
        write_log(site->logger->eventLOG,
            ssprintf("Приоткроем заслонку %d\n", curr_pos));
        site->th->time_start = time(NULL);
      }
    } else {
      //printf("Призакроем заслонку %d\n", curr_pos);
      if (curr_pos != 0) {
        curr_pos--;
        site->th->set_position(site->th, curr_pos);
        write_log(site->logger->eventLOG,
            ssprintf("Призакроем заслонку %d\n", curr_pos));
        site->th->time_start = time(NULL);
      }
    }
  }

  //site->th->time_start = time(NULL);
  site->temp_in_prev = site->temp_in;

  //возврат в основной цикл

}

int sub_uvo_fail(Site* site) {

  write_log(site->logger->eventLOG, ssprintf("sub_uvo_fail\n"));
  float temp_fail = strtof(getStr(site->cfg, (void *) "temp_fail"), NULL);

  if ((site->temp_in) >= temp_fail) {
    // прерываем цикл
    // переходим в аварийный режим охлаждения
    site_mode_fail_temp(site);
  } else {
    // температура нормальная
    float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"),
    NULL);

    write_log(site->logger->eventLOG,
        ssprintf("sub_uvo_fail site->temp_in = %f temp_support = %f\n",
            site->temp_in, temp_support));

    if ((site->temp_in >= (temp_support - 2))
        && (site->temp_in <= (temp_support) + 2)) {
      write_log(site->logger->eventLOG,
          ssprintf(
              "Температура в пределах site->temp_in >= temp_support - 2 И site->temp_in <= temp_support + 2\n"));
      write_log(site->logger->eventLOG, ssprintf("Перейдем sub_uvo_vent\n"));
      //sub_uvo_vent(site);
      // передадим исполнение в основную ветку
      return EXIT_FAILURE;
    } else {
      write_log(site->logger->eventLOG,
          ssprintf("Температура не в пределах\n"));
      write_log(site->logger->eventLOG, ssprintf("Перейдем sub_uvo_pen\n"));
      return sub_uvo_pen(site);
    }
  }
}

/* Режим охлаждения кондиционером */
int site_mode_ac(Site* site) {
  //проверил основные моменты
  write_log(site->logger->eventLOG,
      ssprintf("Режим охлаждения кондиционером!\n"));

  //write_log(site->logger->eventLOG, "Режим охлаждения кондиционером");

  site->mode = 2;
  site->time_pre = time(NULL);

  int a, v, a_cond;
  for (v = 0; v < 2; v++) {
    site->vents[v]->time_start = 0;
  }

  for (a = 0; a < 2; a++) {
    site->acs[a]->is_diff = 0;
  }

  int num_ac = atoi(getStr(site->cfg, (void *) "num_ac"));

  int num_ac_tmp = num_ac;

  write_log(site->logger->eventLOG,
      ssprintf(
          "Количество кондиционеров num_ac_tmp = %d is_diff_0 = %d is_diff_1 = %d\n",
          num_ac_tmp, site->acs[0]->is_diff, site->acs[1]->is_diff));

  for (a = 0; a < 2; a++) {
    site->acs[a]->set_mode(site->acs[a], 1);
    site->acs[a]->time_start = time(NULL);
    write_log(site->logger->eventLOG,
        ssprintf("Включили кондиционер КОНД_%d время включения %d\n", a,
            site->acs[a]->time_start));
  }

  write_log(site->logger->eventLOG,
      ssprintf("До while Режим охлаждения кондиционером\n"));
  while (1) { // sensors was read - ok

    int ret;
    ret = read_sensors(site);
    if (ret != 0) {
      //Ошибка чтения датчиков
      site_mode_fail_ac(site);
    }

    if (difftime(time(NULL), site->time_pre) <= 5) { //секунды
      usleep(100000);
      continue;
    } else {
      write_log(site->logger->eventLOG,
          ssprintf(
              "*************Принятие решения Режим охлаждения кондиционером***************\n"));
      site->time_pre = time(NULL);

      for (a_cond = 0; a_cond < num_ac; a_cond++) { //TODO количество кондиционеров брать из конфига
        if (((site->temp_in - site->acs[a_cond]->temp) >= 10)
            && (site->acs[a_cond]->mode == 1)) {
          //да КОНД_0 Набрал дельту
          write_log(site->logger->eventLOG,
              ssprintf("Набрал дельту КОНД_%d %f дельта %f\n", a_cond,
                  site->acs[a_cond]->temp,
                  (site->temp_in - site->acs[a_cond]->temp)));
          if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
            // да
            write_log(site->logger->eventLOG,
                ssprintf("выключим вентиляцию\n"));
            //выключим вентиляцию
            for (v = 0; v < 2; v++) {
              if (site->vents[v]->mode == 1)
                site->vents[v]->set_step(site->vents[v], 0);
            }

            //site->acs[a_cond]->mode == 1;
            //TODO: Проверить по описанию
            //for (a = 0; a < 2; a++) {
            //  if (site->acs[a]->mode == 1)
            //    site->acs[a]->set_mode(site->acs[a], 0);
            //  site->acs[a]->time_start = time(NULL);
            //}

          }

          if (site->th->position == 10) {
            site->th->set_position(site->th, 0);
            site->th->time_start = time(NULL);
          }

          site->acs[a_cond]->time_start = time(NULL);
          site->acs[a_cond]->is_diff = 1;

        } else {
          //нет КОНД_x не набрал пока дельту
          write_log(site->logger->eventLOG,
              ssprintf(
                  " КОНД_%d не набрал пока дельту temp_in %f site->acs[a_cond]->temp %f \n",
                  a_cond, site->temp_in, site->acs[a_cond]->temp));
          site->acs[a_cond]->is_diff = 0;
        }

        //600
        // отработан промежуток?
        write_log(site->logger->eventLOG, ssprintf("Цикл КОНД_%d\n", a_cond));
        //printf("600 отработан промежуток? diff %d time_start_%d %d\n",(difftime(time(NULL), site->acs[a_cond]->time_start)), a_cond, site->acs[a_cond]->time_start);
        write_log(site->logger->eventLOG,
            ssprintf(
                "600 отработан промежуток? time_start_%d %d time %d diff %d %f\n",
                a_cond, site->acs[a_cond]->time_start, time(NULL),
                time(NULL) - site->acs[a_cond]->time_start,
                difftime(time(NULL), site->acs[a_cond]->time_start)));
        if ((difftime(time(NULL), site->acs[a_cond]->time_start) > 600) //600 для тестов 60
        && (site->acs[a_cond]->mode == 1)) {
          //дельта набрана?
          write_log(site->logger->eventLOG,
              ssprintf(
                  "ДА 600 прошло дельта набрана? time %d - time_start %d = %f is_diff = %d\n",
                  time(NULL), site->acs[a_cond]->time_start,
                  (difftime(time(NULL), site->acs[a_cond]->time_start)),
                  site->acs[a_cond]->is_diff));
          if (site->acs[a_cond]->is_diff == 0) {
            write_log(site->logger->eventLOG,
                ssprintf("Авария кондиционера num_ac_tmp = %d\n", num_ac_tmp));
            // Авария кондиционера
            num_ac_tmp--;
            write_log(site->logger->eventLOG,
                ssprintf("num_ac_tmp = %d", num_ac_tmp));
            // Выключим нерабочий кондиционер
            write_log(site->logger->eventLOG,
                ssprintf("Выключим нерабочий кондиционер КОНД_%d\n", a_cond));
            if (site->acs[a_cond]->mode == 1) {
              site->acs[a_cond]->set_mode(site->acs[a_cond], 0);
              site->acs[a_cond]->mode = 0;
            }
          }
        } //600
      } //for

      write_log(site->logger->eventLOG,
          ssprintf("еще есть живые кондиционеры? num_ac_tmp = %d\n",
              num_ac_tmp));
      // еще есть живые кондиционеры?
      if (num_ac_tmp > 0) {
        write_log(site->logger->eventLOG,
            ssprintf("Да еще есть кондиционеры num_ac_tmp = %d\n", num_ac_tmp));
        //да
        float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"),
        NULL);

        float temp_fail = strtof(getStr(site->cfg, (void *) "temp_fail"), NULL);

        if ((temp_support - site->temp_out) > (site->penalty + 2)) {
          write_log(site->logger->eventLOG,
              ssprintf(
                  "Температура позволяет перейти на УВО переходим в УВО temp_support %f site->temp_out %f site->penalty %d \n",
                  temp_support, site->temp_out, site->penalty));
          // переходим в УВО
          site_mode_uvo(site);

        } else {
          if (site->temp_in >= temp_fail) {
            write_log(site->logger->eventLOG,
                ssprintf(
                    "Температура выше аварийной переходим в аварийный режим site->temp_in %f temp_fail %f\n",
                    site->temp_in, temp_fail));
            //переходим в аварийный режим
            site_mode_fail_ac(site);
          } else {
            write_log(site->logger->eventLOG, ssprintf("работа нормальная\n"));
            // работа нормальная
            continue;
          }
        }
      } else {
        write_log(site->logger->eventLOG,
            ssprintf("НЕТ живых кондиционеров переходим в аварийный режим\n"));
        // авария кондиционирования
        // переходим на УВО
        site_mode_fail_ac(site);
      }
    }
  }

  return 1;
}

/* Режим догрева сайта */
int site_mode_heat(Site* site) {

  write_log(site->logger->eventLOG, ssprintf("Режим догрева сайта!\n"));
  //write_log(site->logger->eventLOG, "Режим догрева сайта"); //падает при повторном вызове
  site->mode = 3;

  int a, v;

  site->time_pre = time(NULL);
  site->th_check = 0;

  for (v = 0; v < 2; v++) {
    site->vents[v]->time_start = 0;
    site->vents[v]->error = ERROR_HEAT;
  }

  write_log(site->logger->eventLOG,
      ssprintf("Проверим положение заслонки %d\n", site->th->position));
  if (site->th->position > 1) {
    // улица
    site->th->set_position(site->th, 0);
    site->th->time_start = time(NULL);
  }
  //сайт

  int i, th_pos_read;
  if (site->th_r_exists) {
    while (i < 300) {
      int ret;
      ret = read_sensors(site);
      if (ret != 0) {
        //Ошибка чтения датчиков
        site_mode_fail_uvo(site);
      }

      if (((time(NULL) - site->th->time_start) >= 30) && site->th_r_exists) {
        if (difftime(time(NULL), site->th->time_start) >= 30) {
          write_log(site->logger->eventLOG,
              ssprintf(
                  "Настало время проверить перевилась ли заслонка site->th_check = %d\n",
                  site->th_check));
          //да Необходимо проверить перевилась ли заслонка
          // не было проверки закслонки
          write_log(site->logger->eventLOG,
              ssprintf("site->th->position = %d th_pos_read = %d",
                  site->th->position, th_pos_read));
          if (site->th->position == th_pos_read) {
            write_log(site->logger->eventLOG,
                ssprintf("да заслонка исправна\n"));
            break;
          } else {
            write_log(site->logger->eventLOG,
                ssprintf("заслонка не исправна\n"));
            site_mode_fail_uvo(site);
          }
        }
        i++;
        sleep(1);
      }
    }
  }

  write_log(site->logger->eventLOG,
      ssprintf("Перед ЦИКЛ ДОГРЕВ time %d site->time_pre %d diff %f\n",
          time(NULL), site->time_pre, difftime(time(NULL), site->time_pre)));
  while (1) {
    // читаем датчики
    int ret;
    ret = read_sensors(site);
    if (ret != 0) {
      //Ошибка чтения датчиков
      site_mode_fail_uvo(site);
    }

    if (difftime(time(NULL), site->time_pre) <= 2) //30
        { //секунды
      usleep(10000);
      //printf("ЦИКЛ time %d site->time_pre %d diff %f\n", time(NULL), site->time_pre, difftime(time(NULL), site->time_pre));
      continue;
    } else {
      write_log(site->logger->eventLOG,
          ssprintf(
              "*****************Принятие решения ДОГРЕВ*******************\n"));
      site->time_pre = time(NULL);

      float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"),
      NULL);

      if (site->temp_in >= temp_support - 4) { //TODO: необходимо ввести параметр до какой температуры греть
                                               //да
        write_log(site->logger->eventLOG,
            ssprintf("Сайт нагрели %f\n выключим ТЭН temp_mix %f",
                site->temp_in, site->temp_mix));
        set_ten(site, 0);

        if (site->temp_mix > 60) {
          write_log(site->logger->eventLOG,
              ssprintf("ТЭН не остыл еще продолжим вентиляцию\n"));
          continue; // продолжаем цикл догрева
        } else {
          write_log(site->logger->eventLOG,
              ssprintf("ТЭН остыл выключим вентиляцию\n"));
          site->vents[0]->set_step(site->vents[0], 0); // обороты приточного

          //выключим вентиляцию
          //for (v = 0; v < 2; v++) {
          //  if (site->vents[v]->mode == 1)
          //    site->vents[v]->set_mode(site->vents[v], 0);
          //}
          write_log(site->logger->eventLOG,
              ssprintf("Перейдем в режим охлаждения УВО\n"));
          site_mode_uvo(site); // режим охлаждения уво
        }
      } else {
        //нет
        write_log(site->logger->eventLOG, ssprintf("Нужно греть сайт\n"));
        // TODO: Проверить по описанию
        if (site->vents[0]->mode == 0) {
          site->vents[0]->set_mode(site->vents[0], 1);
          site->vents[0]->set_step(site->vents[0], 10);
          site->vents[0]->time_start = time(NULL);
        }

        if (difftime(time(NULL), site->vents[0]) > 30) {
          write_log(site->logger->eventLOG,
              ssprintf("Проверим вращается ли вентилятор\n"));
          //да

          if ((site->vents[0]->step == site->tacho1)
              || (site->tacho1_exists != 1)) {
            write_log(site->logger->eventLOG, ssprintf("Да вращается\n"));
            //да
            site->vents[0]->time_start = time(NULL);
            if ((site->ten == 1) && site->vents[0]->error == ERROR_HEAT) {
              write_log(site->logger->eventLOG,
                  ssprintf("Пока неизвестно вращается ли вентилятор\n"));
              //да
              continue;

            } else {
              //нет
              write_log(site->logger->eventLOG,
                  ssprintf("Да вращается включим ТЭН\n"));
              set_ten(site, 1);
              continue;
            }
          } else {
            write_log(site->logger->eventLOG,
                ssprintf(
                    "АВАРИЯ вентилятора Авария вентиляции - охлаждение кондиционером\n"));
            //нет
            set_ten(site, 0);
            site->vents[0]->set_step(site->vents[0], 0);
            site->vents[0]->error = ERROR;

            site_mode_ac(site); // Авария вентиляции - охлаждение кондиционером
          }
        } else {
          write_log(site->logger->eventLOG,
              ssprintf(
                  "Время проверки работает ли вентилятор еще не настало\n"));
          //нет
          //if (((site->ten == 1) && site->vents[0]->error == ERROR_HEAT)
          //    || ((site->ten == 1) && site->vents[1]->error == ERROR_HEAT)) {
          //
          //  //да
          //  continue;

          //} else {
          //нет
          //  site->set_ten(site, 1);
          //  continue;

          //}
          continue;
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
  write_log(site->logger->eventLOG,
      ssprintf("site_mode_fail_uvo: Режим авария УВО!\n"));
  //write_log(site->logger->eventLOG, "Режим авария УВО");
  site->mode = 4;
  site->time_pre = time(NULL);

  int a, v;
  for (v = 0; v < 2; v++) {
    site->vents[v]->time_start = 0;
  }

  for (a = 0; a < 2; a++) {
    site->acs[a]->is_diff = 0;
  }

  int num_ac = atoi(getStr(site->cfg, (void *) "num_ac"));

  for (a = 0; a < num_ac; a++) {
    site->acs[a]->set_mode(site->acs[a], 1);
    site->acs[a]->time_start = time(NULL);
    write_log(site->logger->eventLOG,
        ssprintf(
            "site_mode_fail_uvo: Включили кондиционер КОНД_%d время включения %d\n",
            a, site->acs[a]->time_start));
  }

  int num_ac_tmp = num_ac;

  while (1) { // sensors was read - ok

    int ret, a_cond;
    ret = read_sensors(site);
    if (ret != 0) {
      //Ошибка чтения датчиков
      continue;
    }

    if (difftime(time(NULL), site->time_pre) <= 5) { //секунды
      usleep(10000);
      continue;
    } else {
      write_log(site->logger->eventLOG,
          ssprintf(
              "site_mode_fail_uvo: ****************Авария УВО работаем на кондиционере принятие решения*******************\n"));
      site->time_pre = time(NULL);

      for (a_cond = 0; a_cond < num_ac; a_cond++) {
        if (((site->temp_in - site->acs[a_cond]->temp) >= 10)
            && (site->acs[a_cond]->mode == 1)) {
          //да
          write_log(site->logger->eventLOG,
              ssprintf(
                  "site_mode_fail_uvo: КОНД_%d набрал дельту site->temp_in = %f site->acs[a_cond]->temp = %f\n",
                  a_cond, site->temp_in, site->acs[a_cond]->temp));
          if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
            //да
            // выключим вентиляцию
            for (v = 0; v < 2; v++) {
              site->vents[v]->set_step(site->vents[v], 0);
            }
            // TODO: Проверить по описанию
            for (a = 0; a < num_ac; a++) {
              //site->acs[a]->set_mode(site->acs[a], 1);
              site->acs[a]->time_start = time(NULL);
            }
          }
        }

        //600
        // отработан промежуток?
        //if ((difftime(time(NULL), site->acs[a]->time_start) > 600)
        //    && (site->acs[a]->mode == 1)) {
        //  //дельта набрана?
        //  if (site->acs[a]->is_diff == 0) {
        //    // Авария кондиционера
        //    num_ac_tmp--;
        //    site->acs[a]->set_mode(site->acs[a], 0);
        //  }
        //}
      }

      //да
      float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"),
      NULL);
      write_log(site->logger->eventLOG,
          ssprintf(
              "Проверим температуру в сайте может попробуем перейти на УВО\n"));
      write_log(site->logger->eventLOG,
          ssprintf("temp_support = %f site->temp_out = %f site->penalty = %d\n",
              temp_support, site->temp_out, site->penalty));
      if ((temp_support - site->temp_out) >= (site->penalty + 2)) {
        write_log(site->logger->eventLOG,
            ssprintf(
                "Да температура на улице позволяет, а в сайте температура ниже поддержания - 2 site->temp_in = %f temp_support = %f\n",
                site->temp_in, temp_support));
        if (site->temp_in < temp_support - 2) {
          // переходим в УВО
          write_log(site->logger->eventLOG, ssprintf("Да сайт позволяет\n"));
          site_mode_uvo(site);
        } else {
          write_log(site->logger->eventLOG,
              ssprintf("Нет сайт не позволяет\n"));
          //продолжаем в текущем режиме
          continue;
        }
      } else {
        //продолжаем в текущем режиме
        continue;
      }
    }
  }

  return 1;
}

/* Авария кондиционеров - Охлаждение УВО */
int site_mode_fail_ac(Site* site) {
  write_log(site->logger->eventLOG, ssprintf("Режим авария кондиционеров!\n"));
  //write_log(site->logger->eventLOG, "Режим авария кондиционеров");
  site->mode = 5;

  site->time_pre = time(NULL);
  site->th_check = 0;
  site->th->time_start = 0;

  int a, v;
  float temp_dew = strtof(getStr(site->cfg, (void *) "temp_dew"), NULL);

  if (site->th->position != 10) {
    if (site->temp_out > temp_dew) {
      site->th->set_position(site->th, 10);
      site->th->time_start = time(NULL);
    }
  }

  while (1) {

    int th_pos_read;
    int vents_r[2];

    int ret;
    ret = read_sensors(site);
    if (ret != 0) {
      //Ошибка чтения датчиков
      for (v = 0; v < 2; v++) {
        site->vents[v]->set_step(site->vents[v], 10); // 100%
        site->vents[v]->time_start = time(NULL);
      }

      continue;

    }

    if (difftime(time(NULL), site->time_pre) <= 30) { //секунды
      continue;
    } else {
      write_log(site->logger->eventLOG,
          ssprintf(
              "********************Принятие решения авария кондиционеров работает УВО****************\n"));
      if ((difftime(time(NULL), site->th->time_start) >= 30)
          && (site->th_check == 0)) {

        if (site->th->position != th_pos_read) {
          //for (v = 0; v < 2; v++) {
          //  site->vents[v]->set_mode(site->vents[v], 1);
          //  site->vents[v]->set_step(site->vents[v], 10); // 100%
          //  site->vents[v]->time_start = time(NULL);
          //  site_mode_uvo(site); // перейдем в режим УВО
          //}

          //} else {
          //  site->th_check = 1;
          //  continue;
        }
      }

      site->time_pre = time(NULL);

      // выключим все кондиционеры
      for (a = 0; a < 2; a++) {
        site->acs[a]->set_mode(site->acs[a], 0);
      }

      write_log(site->logger->eventLOG,
          ssprintf("site_mode_fail_ac: включим вентиляцию\n"));
      // включим вентиляцию

      float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"),
      NULL);

      if ((site->temp_out <= site->temp_in)
          && site->temp_in >= temp_support - 2) {
        for (v = 0; v < 2; v++) {
          if (site->vents[v]->mode == 0) {
            write_log(site->logger->eventLOG,
                ssprintf(
                    "Температура на улице ниже температуры в сайте включим вентиляторы\n"));
            site->vents[v]->set_step(site->vents[v], 10); // 100%
            site->vents[v]->time_start = time(NULL);
          }
        }
      } else {
        for (v = 0; v < 2; v++) {
          if (site->vents[v]->mode == 1) {
            write_log(site->logger->eventLOG,
                ssprintf(
                    "Температура на улице выше температуры в сайте выключим вентиляторы\n"));
            site->vents[v]->set_step(site->vents[v], 0); // 0%
            site->vents[v]->time_start = time(NULL);
          }
        }
      }

      //работаем с вытяжным
      if ((difftime(time(NULL), site->vents[0]->time_start) > 30)) {
        if (site->vents[0]->step == vents_r[0]) {
          site->vents[0]->error = NOERROR;
        } else {
          // Заглушка, можем уйти в бесконечный цикл
          // site_mode_ac(site);
        }
      }

      write_log(site->logger->eventLOG,
          ssprintf(
              "Проверим кондишки если авария по питанию и питание появилось можно перейти\n"));
      // проверяем кондиционеры
      //
      for (a = 0; a < 2; a++) {
        if (site->acs[a]->error == NOPOWER) {
          if (site->power == 1) {
            write_log(site->logger->eventLOG,
                ssprintf("Да питание есть перейдем\n"));
            site_mode_ac(site); // переходим в нормальный режим;
          }
        }
      }

      write_log(site->logger->eventLOG,
          ssprintf("Проверим температура не ниже поддержания - 2\n"));
      if (site->temp_in < temp_support - 2) {

        write_log(site->logger->eventLOG,
            ssprintf("Да ниже выключим вениляцию site->vents[v]->mode = %d\n",
                site->vents[v]->mode));
        // выключим вениляцию
        for (v = 0; v < 2; v++) {
          if (site->vents[v]->mode == 1) {
            site->vents[v]->set_step(site->vents[v], 0);
            site->vents[v]->time_stop = time(NULL);

          }
        }

        //выключим кондиционеры
        //for (a = 0; a < 2; a++) {
        //  site->acs[a]->set_mode(site->acs[a], 0);
        //  site->acs[a]->time_stop = time(NULL);
        //}

        float temp_heat = strtof(getStr(site->cfg, (void *) "temp_heat"), NULL);
        write_log(site->logger->eventLOG,
            ssprintf("Сайт не замерз? site->temp_in = %f temp_heat = %f\n",
                site->temp_in, temp_heat));
        if (site->temp_in < temp_heat) {
          write_log(site->logger->eventLOG,
              ssprintf("Да замерз пойдем греть\n"));
          site_mode_heat(site);
        }

        continue;

      }
      write_log(site->logger->eventLOG,
          ssprintf("Переход на функцию логики управления заслонкой\n"));
      sub_uvo_th(site, 1); // Работа с залонкой
    }
  }
  return 1;
}

/* Превышена температура аварии - Аварийный режим охлаждения*/
int site_mode_fail_temp(Site* site) {

  write_log(site->logger->eventLOG, ssprintf("Общий аварийный режим!\n"));
  // write_log(site->logger->eventLOG, "Общий аварийный режим");
  site->mode = 6;

  if (site->temp_in - site->temp_out) {
    // да
    // Охлаждаем УВО
    site_mode_fail_temp_uvo(site);
  } else {
    // нет
    // Охлаждаем кондиционерами
    site_mode_fail_temp_ac(site);
  }

  return 1;
}

int site_mode_fail_temp_uvo(Site* site) {

  int a, v, ret, res;
  float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"), NULL);
  float temp_dew = strtof(getStr(site->cfg, (void *) "temp_dew"), NULL);

  //printf("Режим охлаждения УВО!\n");
  //write_log(site->logger->eventLOG, "Режим охлаждения УВО");
  site->penalty = 0;

  site->vents[0]->set_step(site->vents[0], 1);
  site->vents[1]->set_step(site->vents[1], 1);

  //По умолчанию: кондиц. выкл.
  for (a = 0; a < 2; a++) {
    site->acs[a]->set_mode(site->acs[a], 0);
  }

  site->temp_in_prev = 0;

  site->time_pre = time(NULL);
  site->time_uvo = time(NULL);

  site->vents[0]->time_start = 0;
  site->vents[1]->time_start = 0;

  // читаем датчики
  ret = read_sensors(site);
  if (ret != 0) {
    //Ошибка чтения датчиков
    site_mode_fail_temp_ac(site);
  }

  //printf("Переведем заслонку site->temp_out = %f temp_dew = %f\n",
  //    site->temp_out, temp_dew);

  if ((site->temp_out) > temp_dew) {
    if (site->th->exist) //Это есть ли заслонка? или есть ли откуда читать
    {
      write_log(site->logger->eventLOG,
          ssprintf("переведем заслонку в положение улица\n"));
      site->th->set_position(site->th, 10); //переведем заслонку в положение улица
      site->th->time_start = time(NULL);
    }
  } else {
    if (site->th->exist) {
      write_log(site->logger->eventLOG,
          ssprintf("переведем заслонку в положение сайт\n"));
      site->th->set_position(site->th, 0); //переведем заслонку в положение сайт
      site->th->time_start = time(NULL);
    }
  }

  //printf("Перед while UVO");
  while (1) {

    int vents_r[2];

    ret = read_sensors(site);
    if (ret != 0) {
      site_mode_fail_temp_ac(site);
    }

    if (difftime(time(NULL), site->time_pre) <= 30) {
      continue;
    }

    //printf("****************Принятие решения Режим УВО*****************\n");
    site->time_pre = time(NULL);
    //printf("site->vents[0]->mode = %d site->vents[1]->mode = %d\n",
    //    site->vents[0]->mode, site->vents[1]->mode);

    if (site->temp_in < (temp_support - 2)) {
      site_mode_fail_uvo(site);
    }

    if ((site->temp_in - site->temp_out) <= 5) {
      sub_uvo_pow(site);
    }

    if (site->acs[0]->mode == 1 || site->acs[1]->mode == 1) {
      write_log(site->logger->eventLOG, ssprintf("Выключим кондиционеры\n"));
      for (a = 0; a < 2; a++) {
        site->acs[a]->set_mode(site->acs[a], 0);
      }
    }

    if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
      if (time(NULL) - site->vents[0]->time_start > 30) {
        if (site->vents[0]->step == vents_r[0]) {
          for (v = 0; v < 2; v++) {
            site->vents[v]->error = NOERROR;
          }
        } else {
          for (v = 0; v < 2; v++) {
            site->vents[v]->error = ERROR;
          }
          site_mode_fail_temp_ac(site);
        }
      }
    } else {

      site->time_uvo = time(NULL);
      site->temp_in_prev = site->temp_in;

      for (v = 0; v < 2; v++) {
        site->vents[v]->set_step(site->vents[v], 10);
        site->vents[v]->time_start = time(NULL);
      }
    }
    sub_uvo_th(site, 1);
  }

  return 1;
}

int site_mode_fail_temp_ac(Site* site) {

  write_log(site->logger->eventLOG,
      ssprintf("Авария по температуре: режим охлаждения кондиционером!\n"));

  //write_log(site->logger->eventLOG, "Режим охлаждения кондиционером");
  site->time_pre = time(NULL);

  int a, v, a_cond;
  for (v = 0; v < 2; v++) {
    site->vents[v]->time_start = 0;
  }

  for (a = 0; a < 2; a++) {
    site->acs[a]->is_diff = 0;
  }

  int num_ac = atoi(getStr(site->cfg, (void *) "num_ac"));
  int num_ac_tmp = num_ac;
  int temp_support = strtof(getStr(site->cfg, (void *) "temp_support"), NULL);

  //printf(
  //    "Количество кондиционеров num_ac_tmp = %d is_diff_0 = %d is_diff_1 = %d\n",
  //    num_ac_tmp, site->acs[0]->is_diff, site->acs[1]->is_diff);

  if (site->th->position == 10) {
    site->th->set_position(site->th, 0);
    site->th->time_start = time(NULL);
  }

  for (a = 0; a < 2; a++) {
    site->acs[a]->set_mode(site->acs[a], 1);
    site->acs[a]->time_start = time(NULL);
    //printf("Включили кондиционер КОНД_%d время включения %d\n", a,
  }

//printf("До while Режим охлаждения кондиционером\n");
  while (1) {

    int ret;
    ret = read_sensors(site);
    if (ret != 0) {
      //Ошибка чтения датчиков
      //site_mode_fail_ac(site);
      continue;
    }

    if (difftime(time(NULL), site->time_pre) <= 30) { //секунды
      usleep(100000);
      continue;
    } else {
      write_log(site->logger->eventLOG,
          ssprintf(
              "*************Принятие решения Режим охлаждения кондиционером***************\n"));
      site->time_pre = time(NULL);

      if (site->temp_in < temp_support - 2) {
        site_mode_uvo(site);
      }

      for (a_cond = 0; a_cond < num_ac; a_cond++) { //TODO количество кондиционеров брать из конфига
        if (((site->temp_in - site->acs[a_cond]->temp) >= 5)
            && (site->acs[a_cond]->mode == 1)) {
          //да КОНД_0 Набрал дельту
          //printf("Набрал дельту КОНД_%d %f дельта %f\n", a_cond,
          //    site->acs[a_cond]->temp,
          //    (site->temp_in - site->acs[a_cond]->temp));
          if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
            // да
            write_log(site->logger->eventLOG,
                ssprintf("выключим вентиляцию\n"));
            //выключим вентиляцию
            for (v = 0; v < 2; v++) {
              if (site->vents[v]->mode == 1)
                site->vents[v]->set_step(site->vents[v], 0);
            }
          }
          site->acs[a_cond]->time_start = time(NULL);
          site->acs[a_cond]->is_diff = 1;

        } else {
          //нет КОНД_x не набрал пока дельту
          write_log(site->logger->eventLOG,
              ssprintf(
                  " КОНД_%d не набрал пока дельту temp_in %f site->acs[a_cond]->temp %f \n",
                  a_cond, site->temp_in, site->acs[a_cond]->temp));
          site->acs[a_cond]->is_diff = 0;
        }

        //600
        // отработан промежуток?
        write_log(site->logger->eventLOG, ssprintf("Цикл КОНД_%d\n", a_cond));
        //printf("600 отработан промежуток? diff %d time_start_%d %d\n",(difftime(time(NULL), site->acs[a_cond]->time_start)), a_cond, site->acs[a_cond]->time_start);
        write_log(site->logger->eventLOG,
            ssprintf(
                "600 отработан промежуток? time_start_%d %d time %d diff %d %f\n",
                a_cond, site->acs[a_cond]->time_start, time(NULL),
                time(NULL) - site->acs[a_cond]->time_start,
                difftime(time(NULL), site->acs[a_cond]->time_start)));
        if ((difftime(time(NULL), site->acs[a_cond]->time_start) > 60) //600 для тестов 60
        && (site->acs[a_cond]->mode == 1)) {
          //дельта набрана?
          write_log(site->logger->eventLOG,
              ssprintf(
                  "ДА 600 прошло дельта набрана? time %d - time_start %d = %f is_diff = %d\n",
                  time(NULL), site->acs[a_cond]->time_start,
                  (difftime(time(NULL), site->acs[a_cond]->time_start)),
                  site->acs[a_cond]->is_diff));
          if (site->acs[a_cond]->is_diff == 0) {
            write_log(site->logger->eventLOG,
                ssprintf("Авария кондиционера num_ac_tmp = %d\n", num_ac_tmp));
            // Авария кондиционера
            num_ac_tmp--;
            write_log(site->logger->eventLOG,
                ssprintf("num_ac_tmp = %d", num_ac_tmp));
            // Выключим нерабочий кондиционер
            write_log(site->logger->eventLOG,
                ssprintf("Выключим нерабочий кондиционер КОНД_%d\n", a_cond));
            if (site->acs[a_cond]->mode == 1) {
              site->acs[a_cond]->set_mode(site->acs[a_cond], 0);
              site->acs[a_cond]->mode = 0;
            }
          }
        } //600
      } //for

      write_log(site->logger->eventLOG,
          ssprintf("еще есть живые кондиционеры? num_ac_tmp = %d\n",
              num_ac_tmp));
      // еще есть живые кондиционеры?
      if (num_ac_tmp > 0) {
        if ((site->temp_in - site->temp_out) > 6) {
          site_mode_fail_temp_uvo(site);
        } else {
          continue;
        }
      } else {
        // printf("НЕТ живых кондиционеров переходим в аварийный режим\n");
        // авария кондиционирования
        // переходим на УВО
        site_mode_fail_ac(site);
      }
    }
  }

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
      site_mode_fail_temp(site);
  }

  return 0;
}

int set_ten(Site* site, int val) {

  i2cOpen();

  int addr, value, bit = 0;
  addr = strtol(getStr(site->cfg, "a_relay"), NULL, 16);
  write_log(site->logger->eventLOG, ssprintf("Изменим тен\n"));
  char buf[1];

  if (ioctl(g_i2cFile, I2C_SLAVE, addr) < 0) {
    write_log(site->logger->eventLOG,
        ssprintf("Failed to acquire bus access and/or talk to slave.\n"));
  }

  if (read(g_i2cFile, buf, 1) != 1) {
    write_log(site->logger->eventLOG, ssprintf("Error reading from i2c\n"));
  }

  value = (int) buf[0];

  if ((val == 1) || (val == 0)) {
    if (val == 1)
      value &= ~(1 << bit); // очистим бит
    else
      value |= (1 << bit); // установим бит

    //printf("Управляем регистром, адрес %x, значение %d, %x , бит %d \n", addr,
    //    val, value, bit);
    set_i2c_register(g_i2cFile, addr, value, value);
    site->ten = val;
    i2cClose();
    return 1;
  } else {
    // wrong value
    // неправильное значение
    return 0;
  }
  return 0;
}

int read_sensors(Site* site) {

  OWNET_HANDLE conn = site->conn;
  char *mnt = site->mount_point;

  char *s_temp_out = getStr(site->cfg, (void *) "s_temp_outdoor");
  char *s_temp_in = getStr(site->cfg, (void *) "s_temp_indoor");
  char *s_temp_mix = getStr(site->cfg, (void *) "s_temp_mix");
  char *s_temp_evapor1 = getStr(site->cfg, (void *) "s_temp_evapor1");
  char *s_temp_evapor2 = getStr(site->cfg, (void *) "s_temp_evapor2");

  char *a_tacho_in = getStr(site->cfg, (void *) "a_tacho_flow_in");
  char *a_tacho_out = getStr(site->cfg, (void *) "a_tacho_flow_out");
  char *a_th_adc = getStr(site->cfg, (void *) "a_throttle_adc");

  site->temp_out = get_data(conn, mnt, s_temp_out, 100);
  site->temp_in = get_data(conn, mnt, s_temp_in, 100);
  site->temp_mix = get_data(conn, mnt, s_temp_mix, 100);
  site->temp_evapor1 = get_data(conn, mnt, s_temp_evapor1, 100);
  site->temp_evapor2 = get_data(conn, mnt, s_temp_evapor2, 100);

  site->acs[0]->temp = site->temp_evapor1;
  site->acs[1]->temp = site->temp_evapor2;
//printf("temp_out = %2.2f\n", site->temp_out);
//printf("temp_in = %2.2f\n", site->temp_in);
//printf("temp_mix = %2.2f\n", site->temp_mix);
//printf("temp_evapor1 = %2.2f\n", site->temp_evapor1);
//printf("temp_evapor2 = %2.2f\n", site->temp_evapor2);

//site->tacho1 = i2c_get_tacho_step(site->vents[0],
//    strtol(a_tacho_in, NULL, 16));
//site->tacho2 = i2c_get_tacho_step(site->vents[1],
//    strtol(a_tacho_out, NULL, 16));

  i2c_get_tacho(strtol(a_tacho_in, NULL, 16), strtol(a_tacho_out, NULL, 16));

  site->th_r = i2c_get_th_data(strtol(a_th_adc, NULL, 16));

  return 0;
}

//TODO: Переписать в виде синглетона
//TODO: Добавить мьютексы для многопоточного доступа к переменным
Site* site_new(char* filename) {
  //printf("Попытаемся создать сайт\n");
  Site* site = malloc(sizeof(Site));
//initialize hardware
  int i;
  for (i = 0; i < 2; i++) {
    site->acs[i] = ac_new(i);
    site->vents[i] = vent_new();
  }

// Укажем конкретный тип для вентилятора
  site->vents[0]->type = 0; // приточный
  site->vents[1]->type = 1;

  site->th = throttle_new();

  site->penalty = 0;
  site->temp_in_prev = 0;
  site->conn = NULL;

// Для тестирования на эмуляторе
//site->mount_point = "/mnt/1wire/";
  site->mount_point = "/bus.0/";
  //printf("Попытаемся считать конфиг\n");
  site->cfg = read_config(filename);

  site->set_mode = set_mode;
  site->set_ten = set_ten;
//site->get_ac_time_work = get_time_work;
  //printf("Попытаемся создать журнал\n");
  site->logger = create_logger();

  return site;
}
