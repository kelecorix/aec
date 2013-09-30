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

  log2("Начало работы\n");

  // Установим значение регистра реле в 0
  i2cOpen();
  int addr = strtol(getStr(site->cfg, "a_relay"), NULL, 16);
  log4("Управляем регистром, адрес %x\n", addr);
  set_i2c_register(g_i2cFile, addr, 0xFF, 0xFF);
  i2cClose();

  // По умолчанию
  site->th->set_position(site->th, 0);
  site->th_r_exists = 0;
  site->tacho1_exists = 1;
  site->tacho2_exists = 1;

  //остановим вентиляторы
  site->vents[0]->set_step(site->vents[0], 0);
  site->vents[1]->set_step(site->vents[1], 0);

  //По умолчанию: кондиц. вкл.
  int a;
  for (a = 0; a < site->num_ac; a++) {
    site->acs[a]->set_mode(site->acs[a], 1);
  }

  site_mode_uvo(site);
  //site_mode_fail_uvo(site);
  //site_mode_fail_ac(site);
}

/* Режим охлаждения УВО */
int site_mode_uvo(Site* site) {
  log1("Режим охлаждения УВО!\n");
  logD(site->logger->dataLOG, 0, "Режим охлаждения УВО!");
  site->mode = 1;
  site->time_pre = time(NULL);
  site->time_uvo = time(NULL);

  int ret, res;
  float temp_dew = strtof(getStr(site->cfg, (void *) "temp_dew"), NULL);
  printf("до датчиков\n");
  // читаем датчики
  ret = read_sensors(site);
  if (ret != 0) {
    //Ошибка чтения датчиков
    printf("Ошибка чтения датчиков\n");
    site_mode_fail_uvo(site);
  }
  printf("После чтения датчиков\n");
  log3("Переведем заслонку site->temp_out = %f temp_dew = %f\n", site->temp_out, temp_dew);
  printf("Переведем заслонку site->temp_out = %f temp_dew = %f\n", site->temp_out, temp_dew);
  if ((site->temp_out) > temp_dew) {
    if (site->th->exist) //Это есть ли заслонка? или есть ли откуда читать
    {
      log3("переведем заслонку в положение улица\n");
      site->th->set_position(site->th, 10); //переведем заслонку в положение улица
      site->th->time_start = time(NULL);
    }
  } else {
    if (site->th->exist) {
      log3("переведем заслонку в положение сайт\n");
      site->th->set_position(site->th, 0); //переведем заслонку в положение сайт
      site->th->time_start = time(NULL);
    }
  }

  log3("Перед while UVO\n");
  while (1) {
    ret = read_sensors(site);
    if (ret != 0)
      site_mode_fail_uvo(site);

    site->power = 1;

    if (difftime(time(NULL), site->time_pre) <= 5){
      sleep(1);
      continue;
    }

    //log2("****************Принятие решения Режим УВО*****************\n");
    site->time_pre = time(NULL);
    //log3("site->vents[0]->mode = %d site->vents[1]->mode = %d\n",
    //site->vents[0]->mode, site->vents[1]->mode);
    if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
      log3("Вентиляторы включены принятие решения\n");
      log3("Время разница: time(NULL) - site->time_uvo =  %d\n", time(NULL) - site->time_uvo);

      float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"), NULL);

      log3("site->temp_in <= temp_support %f <= %f\n",site->temp_in,temp_support);

      if (((site->vents[0]->step == 10 || site->vents[1]->step == 10) && ((time(NULL) - site->time_uvo) >= 300))
          || (site->temp_in <= temp_support)) {
        log3("Вентиляторы вращаются на максимум и проработали 300 сек\n");
        res = sub_uvo_fail(site); // 1 - EXIT_FAILURE - не попали в ошибку
        if (res){
          sub_uvo_vent(site);
        } else {
          continue;
        }
      } else {
        //log3("вентиляторы не на максимуме или не прошло 300 сек\n");
        sub_uvo_vent(site);
      }
    } else {
      ///log3("Вентиляторы не включены\n");

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
  float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"),
  NULL);
  float temp_heat = strtof(getStr(site->cfg, (void *) "temp_heat"), NULL);

  log3("Среда позваляет работать на вентиляторах?\n");
  if ((temp_support - site->temp_out) >= 2) {
    log3("Да позваляет\n");
    if ((site->acs[0]->mode == 1) || (site->acs[1]->mode == 1)) {
      log3("Кондиционеры были включены выключим\n");
      for (a = 0; a < site->num_ac; a++) {
        site->acs[a]->set_mode(site->acs[a], 0);
      }
    }

    if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
      log3("Вентиляторы включены site->vents[0]->time_start = %d \n", "Вентиляторы включены site->vents[0]->time_start = %d \n",
          site->vents[0]->time_start);
      if ((time(NULL) - site->vents[0]->time_start) > 30) {
        if (site->tacho1_exists && site->tacho2_exists) {
          log3("Проверим тахо site->vents[0]->step = %d "
              "site->tacho1 = %d, обороты %d | site->vents[1]->step "
              "= %d site->tacho2 = %d обороты %d \n", site->vents[0]->step, site->tacho1, site->tacho1_t, site->vents[1]->step,
              site->tacho2, site->tacho2_t);
          //if ((site->vents[0]->step != site->tacho1)
          //    || (site->vents[1]->step != site->tacho2)) {
          if ((site->tacho1 < 6) || (site->tacho2 < 6)) {
            log3("Какой-то вентилятор не вращается\n");
            for (v = 0; v < 2; v++) {
              site->vents[v]->error = ERROR;
            }
            log3("Авария уво переход на аварийное охлаждение кондиционером\n");
            site_mode_fail_uvo(site);
          } else {
            log3("Вентиляторы вращаются\n");
            for (v = 0; v < 2; v++) {
              site->vents[v]->error = NOERROR;
            }
          }
        }
      }

      if ((time(NULL) - site->time_uvo) >= 1800) {
        log3("Сняли пенальти с уво\n");
        site->penalty = 0;
      }

      int curr_step = site->vents[0]->step;
      log3("curr_step = %d site->temp_in_prev = %f site->temp_in = %f\n", "curr_step = %d site->temp_in_prev = %f site->temp_in = %f\n",
          curr_step, site->temp_in_prev, site->temp_in);

      if (site->temp_in_prev != site->temp_in_prev_prev != site->temp_in) {
        if (site->temp_in_prev <= site->temp_in) {
          log3("Добавим обороты curr_step = %d\n", curr_step);
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
          log3("Уменьшим обороты curr_step = %d\n", curr_step);
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
      log3("sub_uvo_th\n");
      sub_uvo_th(site, 0);
    } else {
      log3("sub_uvo_vent Вентиляторы не включены включим\n");
      site->time_uvo = time(NULL);
      //site->temp_in_prev = site->temp_in; //видимо не сдесь так как тут мы только когда включаем

      if ((site->temp_out > 12) || (site->vents[0]->error == ERROR || site->vents[1]->error == ERROR)) {
        for (v = 0; v < 2; v++) {
          site->vents[v]->set_step(site->vents[v], 11);
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
    log3("Температура не позваляет работать на вентиляторах\n");
    sub_uvo_pow(site);
  }
}

int sub_uvo_pen(Site* site) {

  log1("Мы в sub_uvo_pen\n");
  int a, v;
  float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"),
  NULL);
  float temp_heat = strtof(getStr(site->cfg, (void *) "temp_heat"), NULL);

  site->time_uvo = 0;
  site->th->time_start = 0;
  for (v = 0; v < 2; v++) {
    site->vents[v]->time_start = 0;
  }

  if (site->temp_in < (temp_support - 2)) {
    log3("Температура ниже поддержания - 2\n");
    if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
      log4("Выключим вентиляторы\n");
      for (v = 0; v < 2; v++) {
        site->vents[v]->set_step(site->vents[v], 0);
      }
    }

    if ((temp_support - site->temp_out) <= 2) {
      site_mode_ac(site);
    }

    if (site->acs[0]->mode == 1 || site->acs[1]->mode == 1) {
      log3("Выключим кондиционеры\n");
      for (a = 0; a < site->num_ac; a++) {
        site->acs[a]->set_mode(site->acs[a], 0);
      }
    }

    if (site->temp_in < temp_heat) {
      log3("Переходим в режим догрева\n");
      site_mode_heat(site);
    } else {
      return EXIT_SUCCESS; //вернем 0, чтоб перейти обратно в УВО
    }
  } else {
    log3("Температура выше поддержания + 2\n");
    if (site->power == 0) {
      //питания нет
      for (a = 0; a < site->num_ac; a++) {
        site->acs[a]->error = NOPOWER;
      }

      // переходим в режим охлаждения УВО
      // авария на кондиционере
      log3("переходим в режим охлаждения УВО  авария на кондиционере\n");
      site_mode_fail_ac(site);
    } else {

      if (site->temp_out <= site->temp_in) {
        for (v = 0; v < 2; v++) {
          site->vents[v]->set_step(site->vents[v], 11);
        }
      }

      //питание есть
      site->penalty++;

      // переходим в режим охлаждения кондиционером
      log3("переходим в режим охлаждения кондиционером\n");
      site_mode_ac(site);
    }
  }
  log3("Выход из sub_uvo_pen\n");
  return EXIT_SUCCESS;
}

void sub_uvo_pow(Site* site) {

  log2("******sub_uvo_pow*******\n");
  int a, v;

  site->time_uvo = 0;
  site->th->time_start = 0;
  for (v = 0; v < 2; v++) {
    site->vents[v]->time_start = 0;
  }

  if (site->temp_out < site->temp_in) {
    for (v = 0; v < 2; v++) {
      site->vents[v]->set_step(site->vents[v], 11);
    }
  }

  if (site->power == 1) {

    if (site->temp_out <= site->temp_in) {
      for (v = 0; v < 2; v++) {
        site->vents[v]->set_step(site->vents[v], 11);
      }
    }

    site_mode_ac(site);
  } else {
    for (a = 0; a < site->num_ac; a++) {
      site->acs[a]->error = NOPOWER;
    }
    site_mode_fail_ac(site);
  }
}

// fail- флаг работы в аварийном режиме
//
void sub_uvo_th(Site* site, int fail) {

  log2("*********sub_uvo_th**************\n");
  int v;
  float temp_dew = strtof(getStr(site->cfg, (void *) "temp_dew"), NULL);

  log3("sub_uvo_th temp_dew = %f site->temp_out = %f \n", temp_dew, site->temp_out);
  if (site->temp_out > temp_dew) {
    log3("Где заслонка\n");
    // Заслонка выставлена на улицу
    if (site->th->position != 10) {
      log3("Переводим заслонку на улицу\n");
      site->th->set_position(site->th, 10);
    }
  } else {
    //printf("Не настало ли время проверить работает ли заслонка? diff %d\n",
    //    time(NULL) - site->th->time_start);
    log3("Не настало ли время проверить работает ли заслонка? diff %d\n", time(NULL) - site->th->time_start);
    if ((time(NULL) - site->th->time_start) >= 120) {

      log3("Да настало, а есть ли откуда читать?\n");
      site->th->time_start = time(NULL);
      if (site->th_r_exists) {
        log3("site->th->position = %d site->th_r = %d site->th->position_adc = %d\n", site->th->position, site->th_r,
            site->th->position_adc);
        if (site->th->position == site->th_r) {

          int curr_pos = site->th->position;
          if (site->temp_mix >= temp_dew) {
            site->th->set_position(site->th, curr_pos++);
          } else {
            site->th->set_position(site->th, curr_pos--);
          }
        } else {
          log3("Авария заслонки\n");
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

    log3("Температура в миксере %f температура росы %f site->th->position = %d\n", site->temp_mix, temp_dew, site->th->position);
    if (site->temp_mix >= temp_dew) {
      //printf("Приоткроем заслонку %d\n", curr_pos);
      if (curr_pos != 10) {
        curr_pos++;
        site->th->set_position(site->th, curr_pos);
        log3("Приоткроем заслонку %d\n", curr_pos);
        site->th->time_start = time(NULL);
      }
    } else {
      //printf("Призакроем заслонку %d\n", curr_pos);
      if (curr_pos != 0) {
        curr_pos--;
        site->th->set_position(site->th, curr_pos);
        log3("Призакроем заслонку %d\n", curr_pos);
        site->th->time_start = time(NULL);
      }
    }
  }

  //site->th->time_start = time(NULL);
  site->temp_in_prev = site->temp_in;

  //возврат в основной цикл

}

int sub_uvo_fail(Site* site) {

  log3("sub_uvo_fail\n");
  float temp_fail = strtof(getStr(site->cfg, (void *) "temp_fail"),
  NULL);
  float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"),
  NULL);

  if ((site->temp_in) >= temp_fail) {
    // прерываем цикл
    // переходим в аварийный режим охлаждения
    site_mode_fail_temp(site);
  } else {
    // температура нормальная
    log3("sub_uvo_fail site->temp_in = %f temp_support = %f\n", site->temp_in, temp_support);

    if ((site->temp_in >= (temp_support - 2)) && (site->temp_in <= (temp_support) + 2)) {

      log3("Температура в пределах site->temp_in >= temp_support - 2 И site->temp_in <= temp_support + 2\n");
      log3("Перейдем sub_uvo_vent\n");
      //sub_uvo_vent(site);
      // передадим исполнение в основную ветку
      return EXIT_FAILURE;
    } else {

      log3("Температура не в пределах\n");
      log3("Перейдем sub_uvo_pen\n");
      return sub_uvo_pen(site);
    }
  }
  return 1;
}

/* Режим охлаждения кондиционером */
int site_mode_ac(Site* site) {
  //проверил основные моменты

  int delta_gained = 0;// флаг - дельта набрана 0/1

  log3("Режим охлаждения кондиционером!\n");
  logD(site->logger->dataLOG, 0, "Режим охлаждения кондиционером!");
  site->mode = 2;
  site->time_pre = time(NULL);

  int a, v, a_cond;
  for (v = 0; v < 2; v++) {
    site->vents[v]->time_start = 0;
  }

  for (a = 0; a < site->num_ac; a++) {
    site->acs[a]->is_diff = 0;
  }

  int num_ac_tmp = site->num_ac;

  log3("Количество кондиционеров num_ac_tmp = %d is_diff_0 = %d is_diff_1 = %d\n", num_ac_tmp, site->acs[0]->is_diff,
      site->acs[1]->is_diff);

  for (a = 0; a < site->num_ac; a++) {
    site->acs[a]->set_mode(site->acs[a], 1);
    site->acs[a]->time_start = time(NULL);

    log3("Включили кондиционер КОНД_%d время включения %d\n", a, site->acs[a]->time_start);
  }

  log3("До while Режим охлаждения кондиционером\n");
  while (1) { // sensors was read - ok

    int ret;
    ret = read_sensors(site);

    if (ret != 0) {
      //Ошибка чтения датчиков
      site_mode_fail_ac(site);
    }

    if (difftime(time(NULL), site->time_pre) <= 5) { //секунды
      sleep(1);
      continue;
    } else {

      log3("*************Принятие решения Режим охлаждения кондиционером***************\n");
      site->time_pre = time(NULL);

      for (a_cond = 0; a_cond < site->num_ac; a_cond++) { //TODO количество кондиционеров брать из конфига
        if (((site->temp_in - site->acs[a_cond]->temp) >= 10) && (site->acs[a_cond]->mode == 1)) {
          //да КОНД_0 Набрал дельту
          log3("Набрал дельту КОНД_%d %f дельта %d\n", a_cond, site->acs[a_cond]->temp, (site->temp_in - site->acs[a_cond]->temp));
          if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
            log3("выключим вентиляцию\n");
            //выключим вентиляцию
            for (v = 0; v < 2; v++) {
              if (site->vents[v]->mode == 1)
                site->vents[v]->set_step(site->vents[v], 0);
            }
          }

          //перекроем заслонку
          if (site->th->position == 10) {
            site->th->set_position(site->th, 0);
            site->th->time_start = time(NULL);
          }

          site->acs[a_cond]->time_start = time(NULL);
          site->acs[a_cond]->is_diff = 1;
          delta_gained = 1;

        } else {
          //нет КОНД_x не набрал пока дельту
          log3(" КОНД_%d не набрал пока дельту temp_in %f site->acs[a_cond]->temp %f \n", a_cond, site->temp_in, site->acs[a_cond]->temp);
          if(delta_gained == 0)
            site->acs[a_cond]->is_diff = 0;
        }

        //600
        // отработан промежуток?
        log3("Цикл КОНД_%d\n", a_cond);
        log3("600 отработан промежуток? time_start_%d %d time %d diff %d \n", a_cond, site->acs[a_cond]->time_start, time(NULL),
            (time(NULL) - site->acs[a_cond]->time_start));

        if ((difftime(time(NULL), site->acs[a_cond]->time_start) > 600) && (site->acs[a_cond]->mode == 1)) {
          //дельта набрана?

          log3("ДА 600 прошло дельта набрана? time %d - time_start %d = %f is_diff = %d\n", time(NULL), site->acs[a_cond]->time_start,
              (difftime(time(NULL), site->acs[a_cond]->time_start)), site->acs[a_cond]->is_diff);

          if (site->acs[a_cond]->is_diff == 0) {

            log3("Авария кондиционера num_ac_tmp = %d\n", num_ac_tmp);
            // Авария кондиционера
            num_ac_tmp--;

            log3("num_ac_tmp = %d", num_ac_tmp);
            // Выключим нерабочий кондиционер

            log3("Выключим нерабочий кондиционер КОНД_%d\n", a_cond);
            if (site->acs[a_cond]->mode == 1) {
              site->acs[a_cond]->set_mode(site->acs[a_cond], 0);
              site->acs[a_cond]->mode = 0;
            }
          }
        } //600
      } //for

      log3("еще есть живые кондиционеры? num_ac_tmp = %d\n", num_ac_tmp);
      // еще есть живые кондиционеры?
      if (num_ac_tmp > 0) {

        log3("Да еще есть кондиционеры num_ac_tmp = %d\n", num_ac_tmp);
        //да
        float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"),
        NULL);

        float temp_fail = strtof(getStr(site->cfg, (void *) "temp_fail"),
        NULL);

        if (((temp_support - site->temp_out) > (site->penalty + 2)) && (site->temp_in < (temp_support - 1))) {

          log3("Температура позволяет перейти на УВО переходим в УВО temp_support %f site->temp_out %f site->penalty %d \n", temp_support,
              site->temp_out, site->penalty);
          // переходим в УВО
          site_mode_uvo(site);

        } else {
          if (site->temp_in >= temp_fail) {

            log3("Температура выше аварийной переходим в аварийный режим site->temp_in %f temp_fail %f\n", site->temp_in, temp_fail);
            //переходим в аварийный режим
            site_mode_fail_ac(site);
          } else {
            log3("работа нормальная\n");
            // работа нормальная
            continue;
          }
        }
      } else {

        log3("НЕТ живых кондиционеров переходим в аварийный режим\n");
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

  log3("Режим догрева сайта!\n");
  logD(site->logger->dataLOG, 0, "Режим догрева сайта!");
  //    "Режим догрева сайта"); //падает при повторном вызове
  site->mode = 3;

  int a, v;

  site->time_pre = time(NULL);
  site->th_check = 0;

  for (v = 0; v < 2; v++) {
    site->vents[v]->time_start = 0;
    site->vents[v]->error = ERROR_HEAT;
  }

  log3("Проверим положение заслонки %d\n", site->th->position);
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

          log3(("Настало время проверить перевилась ли заслонка site->th_check = %d\n", site->th_check));
          //да Необходимо проверить перевилась ли заслонка
          // не было проверки закслонки

          log3("site->th->position = %d th_pos_read = %d", site->th->position, th_pos_read);
          if (site->th->position == th_pos_read) {

            log3("да заслонка исправна\n");
            break;
          } else {

            log3("заслонка не исправна\n");
            site_mode_fail_uvo(site);
          }
        }
        i++;
        sleep(1);
      }
    }
  }

  log3("Перед ЦИКЛ ДОГРЕВ time %d site->time_pre %d diff %f\n", time(NULL), site->time_pre, difftime(time(NULL), site->time_pre));
  while (1) {
    // читаем датчики
    int ret;
    ret = read_sensors(site);
    if (ret != 0) {
      //Ошибка чтения датчиков
      site_mode_fail_uvo(site);
    }

    if (difftime(time(NULL), site->time_pre) <= 5) //30
    { //секунды
      //printf("ЦИКЛ time %d site->time_pre %d diff %f\n", time(NULL), site->time_pre, difftime(time(NULL), site->time_pre));
      sleep(1);
      continue;
    } else {

      log3("*****************Принятие решения ДОГРЕВ*******************\n");
      site->time_pre = time(NULL);

      float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"),
      NULL);

      if (site->temp_in >= temp_support - 4) { //TODO: необходимо ввести параметр до какой температуры греть
                                               //да

        log3("Сайт нагрели %f\n выключим ТЭН temp_mix %f", site->temp_in, site->temp_mix);
        set_ten(site, 0);

        if (site->temp_mix > 60) {

          log3("ТЭН не остыл еще продолжим вентиляцию\n");
          continue; // продолжаем цикл догрева
        } else {

          log3("ТЭН остыл выключим вентиляцию\n");
          site->vents[0]->set_step(site->vents[0], 0); // обороты приточного

          //выключим вентиляцию
          //for (v = 0; v < 2; v++) {
          //  if (site->vents[v]->mode == 1)
          //    site->vents[v]->set_mode(site->vents[v], 0);
          //}

          log3("Перейдем в режим охлаждения УВО\n");
          site_mode_uvo(site); // режим охлаждения уво
        }
      } else {
        //нет
        log3("Нужно греть сайт\n");
        // TODO: Проверить по описанию
        if (site->vents[0]->mode == 0) {
          //site->vents[0]->set_mode(site->vents[0], 1);
          site->vents[0]->set_step(site->vents[0], 11);
          site->vents[0]->time_start = time(NULL);
        }

        if (difftime(time(NULL), site->vents[0]->time_start) > 120) {

          log3("Проверим вращается ли вентилятор\n");
          //да

          if ((site->vents[0]->step == site->tacho1) || (site->tacho1_exists != 1)) {
            log3("Да вращается\n");
            //да
            site->vents[0]->time_start = time(NULL);
            if ((site->ten == 1) && site->vents[0]->error == ERROR_HEAT) {

              log3("Пока неизвестно вращается ли вентилятор\n");
              //да
              continue;

            } else {
              //нет

              log3("Да вращается включим ТЭН\n");
              set_ten(site, 1);
              continue;
            }
          } else {

            log3("АВАРИЯ вентилятора Авария вентиляции - охлаждение кондиционером\n");
            //нет
            set_ten(site, 0);
            site->vents[0]->set_step(site->vents[0], 0);
            site->vents[0]->error = ERROR;

            site_mode_ac(site); // Авария вентиляции - охлаждение кондиционером
          }
        } else {

          log3("Время проверки работает ли вентилятор еще не настало\n");
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

  log3("site_mode_fail_uvo: Режим авария УВО!\n");
  logD(site->logger->dataLOG, 0, "Режим авария УВО!");
  //    "Режим авария УВО");
  site->mode = 4;
  site->time_pre = time(NULL);

  int a, v;
  for (v = 0; v < 2; v++) {
    site->vents[v]->time_start = 0;
  }

  for (a = 0; a < site->num_ac; a++) {
    site->acs[a]->is_diff = 0;
  }

  for (a = 0; a < site->num_ac; a++) {
    site->acs[a]->set_mode(site->acs[a], 1);
    site->acs[a]->time_start = time(NULL);

    log3("site_mode_fail_uvo: Включили кондиционер КОНД_%d время включения %d\n", a, site->acs[a]->time_start);
  }

  int num_ac_tmp = site->num_ac;

  while (1) { // sensors was read - ok

    int ret, a_cond;
    ret = read_sensors(site);
    if (ret != 0) {
      //Ошибка чтения датчиков
      continue;
    }

    if (difftime(time(NULL), site->time_pre) <= 5) { //секунды
      sleep(1);
      continue;
    } else {

      log3("site_mode_fail_uvo: ****************Авария УВО работаем на кондиционере принятие решения*******************\n");
      site->time_pre = time(NULL);

      for (a_cond = 0; a_cond < site->num_ac; a_cond++) {
        if (((site->temp_in - site->acs[a_cond]->temp) >= 10) && (site->acs[a_cond]->mode == 1)) {
          //да

          log3("site_mode_fail_uvo: КОНД_%d набрал дельту site->temp_in = %f site->acs[a_cond]->temp = %f\n", a_cond, site->temp_in,
              site->acs[a_cond]->temp);
          if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
            //да
            // выключим вентиляцию
            log3("site_mode_fail_uvo: выключим вентиляцию\n");
            
            for (v = 0; v < 2; v++) {
              site->vents[v]->set_step(site->vents[v], 0);
            }
            log3("site_mode_fail_uvo: закроем заслонку\n");
            site->th->set_position(site->th, 0);
            site->th->time_start = time(NULL);

            // TODO: Проверить по описанию
            for (a = 0; a < site->num_ac; a++) {
              //site->acs[a]->set_mode(site->acs[a], 1);
              site->acs[a]->time_start = time(NULL);
            }
          }
        }
      }

      //да
      float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"), NULL);

      log3("Проверим температуру в сайте может попробуем перейти на УВО\n");

      log3("temp_support = %f site->temp_out = %f site->penalty = %d\n", temp_support, site->temp_out, site->penalty);
      if ((temp_support - site->temp_out) >= (site->penalty + 2)) {

        log3("Да температура на улице позволяет, а в сайте температура ниже поддержания - 2 site->temp_in = %f temp_support = %f\n",
            site->temp_in, temp_support);
        if (site->temp_in < temp_support - 2) {
          // переходим в УВО
          log3("Да сайт позволяет\n");
          site_mode_uvo(site);
        } else {

          log3("Нет сайт не позволяет\n");
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
  log3("Режим авария кондиционеров!\n");
  logD(site->logger->dataLOG, 0, "Режим авария кондиционеров!");
  //    "Режим авария кондиционеров");
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
        site->vents[v]->set_step(site->vents[v], 11); // 100%
        site->vents[v]->time_start = time(NULL);
      }
      continue;
    }

    if (difftime(time(NULL), site->time_pre) <= 5) { //секунды
      sleep(1);
      continue;
    } else {

      log3(("********************Принятие решения авария кондиционеров работает УВО****************\n"));
      if ((difftime(time(NULL), site->th->time_start) >= 30) && (site->th_check == 0)) {

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
      for (a = 0; a < site->num_ac; a++) {
        site->acs[a]->set_mode(site->acs[a], 0);
      }

      log3("site_mode_fail_ac: включим вентиляцию\n");
      // включим вентиляцию

      float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"),
      NULL);

      if ((site->temp_out <= site->temp_in) && site->temp_in >= temp_support - 2) {
        for (v = 0; v < 2; v++) {
          if (site->vents[v]->mode == 0) {

            log3("Температура на улице ниже температуры в сайте включим вентиляторы\n");
            site->vents[v]->set_step(site->vents[v], 11); // 100%
            site->vents[v]->time_start = time(NULL);
          }
        }
      } else {
        for (v = 0; v < 2; v++) {
          if (site->vents[v]->mode == 1) {

            log3("Температура на улице выше температуры в сайте выключим вентиляторы\n");
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
          // TODO: Заглушка, можем уйти в бесконечный цикл
          // site_mode_ac(site);
        }
      }

      log3("Проверим кондишки если авария по питанию и питание появилось можно перейти\n");
      // проверяем кондиционеры
      //
      for (a = 0; a < site->num_ac; a++) {
        if (site->acs[a]->error == NOPOWER) {
          if (site->power == 1) {

            if (site->temp_out <= site->temp_in) {
              for (v = 0; v < 2; v++) {
                site->vents[v]->set_step(site->vents[v], 11);
              }
            }

            log3("Да питание есть перейдем\n");
            site_mode_ac(site); // переходим в нормальный режим;
          }
        }
      }

      log3("Проверим температура не ниже поддержания - 2\n");
      if (site->temp_in < temp_support - 2) {

        log3("Да ниже выключим вениляцию site->vents[v]->mode = %d\n", site->vents[v]->mode);
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

        float temp_heat = strtof(getStr(site->cfg, (void *) "temp_heat"),
        NULL);

        log3("Сайт не замерз? site->temp_in = %f temp_heat = %f\n", site->temp_in, temp_heat);
        if (site->temp_in < temp_heat) {

          log3("Да замерз пойдем греть\n");
          site_mode_heat(site);
        }
        continue;
      }

      log3("Переход на функцию логики управления заслонкой\n");
      sub_uvo_th(site, 1); // Работа с залонкой
    }
  }
  return 1;
}

/* Превышена температура аварии - Аварийный режим охлаждения*/
int site_mode_fail_temp(Site* site) {

  log3("site_mode_fail_temp: Авария по температуре!\n");
  logD(site->logger->dataLOG, 0, "Авария по температуре!");
  //     "Общий аварийный режим");
  site->mode = 6;

  log3("site_mode_fail_temp: site->temp_in - site->temp_out = %f >= 5 \n",(site->temp_in - site->temp_out));
  if (site->temp_in - site->temp_out >= 5) {
    // да
    // Охлаждаем УВО
    log3("site_mode_fail_temp: перейдем site_mode_fail_temp_uvo\n");
    site_mode_fail_temp_uvo(site);
  } else {
    // нет
    // Охлаждаем кондиционерами
    log3("site_mode_fail_temp: перейдем site_mode_fail_temp_ac\n");
    site_mode_fail_temp_ac(site);
  }

  return 1;
}

int site_mode_fail_temp_uvo(Site* site) {

  log3("site_mode_fail_temp_uvo: RUN\n");
  logD(site->logger->dataLOG, 0, "Авария по температуре! УВО");
  int a, v, ret, res;
  float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"), NULL);
  float temp_dew = strtof(getStr(site->cfg, (void *) "temp_dew"), NULL);

  //printf("Режим охлаждения УВО!\n");
  //    "Режим охлаждения УВО");
  site->penalty = 0;

  site->vents[0]->set_step(site->vents[0], 11);
  site->vents[1]->set_step(site->vents[1], 11);
  site->vents[0]->time_start = time(NULL);
  site->vents[1]->time_start = time(NULL);
  
  //По умолчанию: кондиц. выкл.
  for (a = 0; a < site->num_ac; a++) {
    site->acs[a]->set_mode(site->acs[a], 0);
  }

  site->temp_in_prev = 0;

  site->time_pre = time(NULL);
  site->time_uvo = time(NULL);

  //site->vents[0]->time_start = 0;
  //site->vents[1]->time_start = 0;

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
      log3("переведем заслонку в положение улица\n");
      site->th->set_position(site->th, 10); //переведем заслонку в положение улица
      site->th->time_start = time(NULL);
    }
  } else {
    if (site->th->exist) {
      log3("переведем заслонку в положение сайт\n");
      site->th->set_position(site->th, 0); //переведем заслонку в положение сайт
      site->th->time_start = time(NULL);
    }
  }

  log3("site_mode_fail_temp_uvo: Перед while\n");
  while (1) {

    int vents_r[2];

    ret = read_sensors(site);
    if (ret != 0) {
      site_mode_fail_temp_ac(site);
    }

    if (difftime(time(NULL), site->time_pre) <= 5) {
      sleep(1);
      continue;
    }

    log3("site_mode_fail_temp_uvo: ****************Принятие решения*****************\n");
    site->time_pre = time(NULL);
    //printf("site->vents[0]->mode = %d site->vents[1]->mode = %d\n",
    //    site->vents[0]->mode, site->vents[1]->mode);

    if (site->temp_in < (temp_support - 2)) {
      site_mode_uvo(site);
    }

    
    if ((site->temp_in - site->temp_out) <= 5) {
      log3("site_mode_fail_temp_uvo: site->temp_in - site->temp_out = %f <= 5\n",(site->temp_in - site->temp_out));
      site->time_uvo = 0;
      site->th->time_start = 0;
      //for (v = 0; v < 2; v++) {
      //  site->vents[v]->time_start = 0;
      //}

      if (site->temp_out <= site->temp_in) {
        
        for (v = 0; v < 2; v++) {
          site->vents[v]->set_step(site->vents[v], 11);
          site->vents[v]->time_start = time(NULL);
        }
      }

      if (site->power == 1) {
        log3("site_mode_fail_temp_uvo: Перейдем в site_mode_fail_temp_ac\n");
        site_mode_fail_temp_ac(site);
      } else {
        for (v = 0; v < 2; v++) {
          site->vents[v]->error = NOPOWER;
        }
        site_mode_fail_temp_uvo(site);
      }
    }

    if (site->acs[0]->mode == 1 || site->acs[1]->mode == 1) {
      log3("site_mode_fail_temp_uvo: Выключим кондиционеры\n");
      for (a = 0; a < site->num_ac; a++) {
        site->acs[a]->set_mode(site->acs[a], 0);
      }
    }

    log3("site_mode_fail_temp_uvo: Включены ли вентиляторы? site->vents[0]->mode = %d site->vents[1]->mode = %d\n",site->vents[0]->mode,site->vents[1]->mode);
    if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
      if (time(NULL) - site->vents[0]->time_start > 180) {
        log3("site_mode_fail_temp_uvo: Проверим вращаются ли вентиляторы\n");
        if (site->vents[0]->step == vents_r[0]) {
          log3("site_mode_fail_temp_uvo: ДА вращаются\n");
          for (v = 0; v < 2; v++) {
            site->vents[v]->error = NOERROR;
          }
        } else {
          log3("site_mode_fail_temp_uvo: НЕТ не вращаются\n");
          //TODO: необходимо обнулить моточасы вентиляторов
          for (v = 0; v < 2; v++) {
            site->vents[v]->error = ERROR;
          }
          log3("site_mode_fail_temp_uvo: Авария вентиляторов перейдем в site_mode_fail_temp_ac\n");
          site_mode_fail_temp_ac(site);
        }
      }
    } else {

      site->time_uvo = time(NULL);
      site->temp_in_prev = site->temp_in;

      for (v = 0; v < 2; v++) {
        site->vents[v]->set_step(site->vents[v], 11);
        site->vents[v]->time_start = time(NULL);
      }
    }
    sub_uvo_th(site, 1);
  }

  return 1;
}

int site_mode_fail_temp_ac(Site* site) {

  log3("Авария по температуре: режим охлаждения кондиционером!\n");
  logD(site->logger->dataLOG, 0, "Авария по температуре! Кондиционеры");
  //    "Режим охлаждения кондиционером");
  site->time_pre = time(NULL);

  int delta_gained = 0;// флаг - дельта набрана 0/1

  int a, v, a_cond;
  for (v = 0; v < 2; v++) {
    site->vents[v]->time_start = 0;
  }

  for (a = 0; a < site->num_ac; a++) {
    site->acs[a]->is_diff = 0;
  }

  int num_ac_tmp = site->num_ac;
  int temp_support = strtof(getStr(site->cfg, (void *) "temp_support"),
  NULL);

  //printf(
  //    "Количество кондиционеров num_ac_tmp = %d is_diff_0 = %d is_diff_1 = %d\n",
  //    num_ac_tmp, site->acs[0]->is_diff, site->acs[1]->is_diff);

  if (site->vents[0]->mode == 0 || site->vents[1]->mode == 0) {
    if (site->th->position == 10) {
      site->th->set_position(site->th, 0);
      site->th->time_start = time(NULL);
    }
  }

  for (a = 0; a < site->num_ac; a++) {
    site->acs[a]->set_mode(site->acs[a], 1);
    site->acs[a]->time_start = time(NULL);
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

    if (difftime(time(NULL), site->time_pre) <= 5) { //секунды
      sleep(1);
      continue;
    } else {

      log3("site_mode_fail_temp_ac: *************Принятие решения Режим охлаждения кондиционером***************\n");
      site->time_pre = time(NULL);

      if (site->temp_in < temp_support - 2) {
        log3("site_mode_fail_temp_ac: Температуру сбросили перейдем в штатный режим охлаждения site_mode_uvo\n");
        site_mode_uvo(site);
      }

      for (a_cond = 0; a_cond < site->num_ac; a_cond++) { //TODO количество кондиционеров брать из конфига
        if (((site->temp_in - site->acs[a_cond]->temp) >= 5) && (site->acs[a_cond]->mode == 1)) {
          //да КОНД_0 Набрал дельту
          log3("Набрал дельту КОНД_%d %f дельта %f\n", a_cond, site->acs[a_cond]->temp, (site->temp_in - site->acs[a_cond]->temp));
          delta_gained = 1;
          if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1) {
            // да

            log3("выключим вентиляцию\n");
            //выключим вентиляцию
            for (v = 0; v < 2; v++) {
              if (site->vents[v]->mode == 1)
                site->vents[v]->set_step(site->vents[v], 0);
            }
            //TODO: закроем заслонку
            log3("Закроем заслонку\n");
            if (site->th->position == 10) {
              site->th->set_position(site->th, 0);
              site->th->time_start = time(NULL);
            }
            
          }
          //site->acs[a_cond]->time_start = time(NULL);
          site->acs[a_cond]->is_diff = 1;
          delta_gained = 1;

        } else {
          //нет КОНД_x не набрал пока дельту
          log3("КОНД_%d не набрал пока дельту temp_in %f site->acs[a_cond]->temp %f \n", a_cond, site->temp_in, site->acs[a_cond]->temp);
          if(delta_gained == 0)
            site->acs[a_cond]->is_diff = 0;
        }

        //600
        // отработан промежуток?
        log3("Цикл КОНД_%d\n", a_cond);
        //log3("600 отработан промежуток? diff %d time_start_%d %d\n",(difftime(time(NULL), site->acs[a_cond]->time_start)), a_cond, site->acs[a_cond]->time_start);

        log3("600 отработан промежуток? time_start_%d %d time %d diff %d %f\n", a_cond, site->acs[a_cond]->time_start, time(
        NULL), time(NULL) - site->acs[a_cond]->time_start, difftime(time(NULL), site->acs[a_cond]->time_start));
        //600 для тестов 60
        log3("before if\n");
        if ((difftime(time(NULL), site->acs[a_cond]->time_start) > 600) && (site->acs[a_cond]->mode == 1)) {
          //дельта набрана?
          log3("after if\n");
          log3("ДА 600 прошло дельта набрана? time %d - time_start %d = %f is_diff = %d\n", time(NULL), site->acs[a_cond]->time_start,
              (difftime(time(NULL), site->acs[a_cond]->time_start)), site->acs[a_cond]->is_diff);

          if (site->acs[a_cond]->is_diff == 0) {

            log3("Авария кондиционера num_ac_tmp = %d\n", num_ac_tmp);
            // Авария кондиционера
            num_ac_tmp--;

            log3("num_ac_tmp = %d", num_ac_tmp);
            // Выключим нерабочий кондиционер

            log3("Выключим нерабочий кондиционер КОНД_%d\n", a_cond);
            if (site->acs[a_cond]->mode == 1) {
              site->acs[a_cond]->set_mode(site->acs[a_cond], 0);
              site->acs[a_cond]->mode = 0;
            }
          }
        } //600
      } //for

      log3("еще есть живые кондиционеры? num_ac_tmp = %d\n", num_ac_tmp);
      // еще есть живые кондиционеры?
      if (num_ac_tmp > 0) {
        log3("да есть живые кондиционеры site->temp_in - site->temp_out = %f > 6 ?\n", (site->temp_in - site->temp_out));
        if (((site->temp_in - site->temp_out) > 6) && (site->vents[0]->error != ERROR && site->vents[1]->error != ERROR)) {
          log3("ДА перейдем в site_mode_fail_temp_uvo\n");
          site_mode_fail_temp_uvo(site);
        } else {
          log3("НЕТ продолжим охлаждать кондиционером\n");                   
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
  log3("Изменим тен\n");
  char buf[1];

  if (ioctl(g_i2cFile, I2C_SLAVE, addr) < 0) {

    log3("Failed to acquire bus access and/or talk to slave.\n");
  }

  if (read(g_i2cFile, buf, 1) != 1) {
    log3("TEN Error reading from i2c\n");
  }

  value = (int) buf[0];

  if ((val == 1) || (val == 0)) {
    if (val == 1)
      value &= ~(1 << bit); // очистим бит
    else
      value |= (1 << bit); // установим бит

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
  printf("начинаем читать датчики\n");
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

  printf("пытаемся записать температуры\n");
  //TODO: переделать более грамотно, а что если
  // у нас 5 кондиционеров
  // Первый читаем в обязательном порядке
  site->acs[0]->temp = site->temp_evapor1;
  if(site->num_ac == 2)
    site->acs[1]->temp = site->temp_evapor2;

  i2c_get_tacho(strtol(a_tacho_in, NULL, 16), strtol(a_tacho_out, NULL, 16));

  site->th_r = i2c_get_th_data(strtol(a_th_adc, NULL, 16));
  printf("считали датчики\n");
  return 0;
}

//TODO: Переписать в виде синглетона
//TODO: Добавить мьютексы для многопоточного доступа к переменным
Site* site_new(char* filename) {

  printf("создадим сайт\n");
  Site* site = malloc(sizeof(Site));
  int i;
  for (i = 0; i < 2; i++) {
    site->vents[i] = vent_new();
  }

  site->vents[0]->type = 0;  // приточный
  site->vents[1]->type = 1;

  site->th = throttle_new();

  site->penalty = 0;
  site->temp_in_prev = 0;
  site->conn = NULL;

  site->mount_point = "/bus.0/";
  site->cfg = read_config(filename);

  site->set_mode = set_mode;
  site->set_ten = set_ten;

  site->logger = create_logger();

  int num_ac = atoi(getStr(site->cfg, (void *) "num_ac"));
  site->acs = malloc(num_ac * sizeof(int*));
  site->num_ac = num_ac;

  for(i=0; i < (site->num_ac); i++){
    site->acs[i] = ac_new(i);
  }
  printf("Количество кондишек: %d", site->num_ac);
  return site;
}
