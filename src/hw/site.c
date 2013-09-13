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
Site* site;

void site_free() {

  //TODO: очистим ресурсы памяти
}

void run(Site* site) {

  write_log(site->logger->eventLOG, "Начало работы");

  // Установим значение регистра реле в 0
  i2cOpen();
  int addr = strtol(getStr(site->cfg, "a_relay"), NULL, 16);
  printf("Управляем регистром, адрес %x\n", addr);
  set_i2c_register(g_i2cFile, addr, 0xFF, 0xFF);
  i2cClose();

  // По умолчанию
  site->th->set_mode(site->th, 0);
  site->th_r_exists = 0;
  site->tacho1_exists = 0;
  site->tacho2_exists = 0;

  site_mode_uvo(site);
}

/* Режим охлаждения УВО */
int site_mode_uvo(Site* site) {
  printf("Режим охлаждения УВО!\n");
  write_log(site->logger->eventLOG, "Режим охлаждения УВО");
  site->mode = 1;
  site->time_pre = time(NULL);
  site->time_uvo = time(NULL);

  int a, ret, res;
  float temp_dew = strtof(getStr(site->cfg, (void *) "temp_dew"), NULL);

  //По умолчанию: кондиц. вкл.
  for (a = 0; a < 2; a++)
  {
    site->acs[a]->set_mode(site->acs[a], 1);
  }

  //остановим вентиляторы
  site->vents[0]->set_turns(site->vents[0], 0);
  site->vents[1]->set_turns(site->vents[1], 0);

  // читаем датчики
  ret = read_sensors(site);
  if (ret != 0)
  {
    //Ошибка чтения датчиков
    site_mode_fail_uvo(site);
  }

  printf("Переведем заслонку site->temp_out = %f temp_dew = %f\n", site->temp_out, temp_dew);
  if ((site->temp_out) > temp_dew)
  {
    if (site->th->exist) //Это есть ли заслонка? или есть ли откуда читать
    {
      printf("переведем заслонку в положение улица\n");
      site->th->set_position(site->th, 10); //переведем заслонку в положение улица
      site->th->time_start = time(NULL);
    }
  }
  else
  {
    if (site->th->exist)
    {
      printf("переведем заслонку в положение сайт\n");
      site->th->set_position(site->th, 0); //переведем заслонку в положение сайт
      site->th->time_start = time(NULL);
    }
  }

  printf("Перед while UVO");
  while (1)
  {
    ret = read_sensors(site);
    if (ret != 0)
    {
      site_mode_fail_uvo(site);
    }

    site->power = 1;

    if (difftime(time(NULL), site->time_pre) <= 5)
    {
      continue;
    }

    printf("*******************Принятие решения Режим УВО********************\n");
    site->time_pre = time(NULL);
    printf("site->vents[0]->mode = %d site->vents[1]->mode = %d\n", site->vents[0]->mode, site->vents[1]->mode);
    if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1)
    {
      printf("Вентиляторы включены принятие решения\n");
      printf("Время разница: %d\n", (time(NULL) - site->time_uvo));
      float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"), NULL);
      if (((site->vents[0]->turns == 10 || site->vents[1]->turns == 10) && ((time(NULL) - site->time_uvo) >= 60))
          || (site->temp_in <= temp_support))
      {
        printf("Вентиляторы вращаются на максимум и проработали 300 сек\n");
        res = sub_uvo_fail(site); // 1 - EXIT_FAILURE - не попали в ошибку
        if (res)
          sub_uvo_vent(site);
        else
          continue;
      }
      else
      {
        printf("вентиляторы не на максимуме или не прошло 300 сек\n");
        sub_uvo_vent(site);
      }
    }
    else
    {
      printf("Вентиляторы не включены\n");

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

  printf("Среда позваляет работать на вентиляторах?\n");
  if ((temp_support - site->temp_out) >= 2)
  {
    printf("Да позваляет\n");
    if ((site->acs[0]->mode == 1) || (site->acs[1]->mode == 1))
    {
      printf("Кондиционеры были включены выключим\n");
      for (a = 0; a < 2; a++)
      {
        site->acs[a]->set_mode(site->acs[a], 0);
      }
    }

    if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1)
    {
      printf("Вентиляторы включены site->vents[0]->time_start = %d \n", site->vents[0]->time_start);
      if ((time(NULL) - site->vents[0]->time_start) > 30)
      {
        if (site->tacho1_exists && site->tacho2_exists)
        {
          printf("Проверим тахо\n");
          if ((site->vents[0]->turns != site->tacho1) || (site->vents[1]->turns != site->tacho2))
          {
            printf("Какойто вентилятор не вращается\n");
            for (v = 0; v < 2; v++)
            {
              site->vents[v]->error = ERROR;
            }
            printf("Авария уво переход на аварийное охлаждение кондиционером\n");
            site_mode_fail_uvo(site);
          }
          else
          {
            printf("Вентиляторы вращаются\n");
            for (v = 0; v < 2; v++)
            {
              site->vents[v]->error = NOERROR;
            }
          }
        }
      }

      if ((time(NULL) - site->time_uvo) >= 1800)
      {
        printf("Сняли пенальти с уво\n");
        site->penalty = 0;
      }

      int curr_turns = site->vents[0]->turns;
      printf("curr_turns = %d site->temp_in_prev = %f site->temp_in = %f\n", curr_turns, site->temp_in_prev, site->temp_in);

      if (site->temp_in_prev != site->temp_in_prev_prev != site->temp_in)
      {
        if (site->temp_in_prev <= site->temp_in)
        {
          printf("Добавим обороты curr_turns = %d\n", curr_turns);
          if (curr_turns != 10)
          {
            curr_turns++;
            if (curr_turns > 10)
            {
              curr_turns = 10;
            }
            for (v = 0; v < 2; v++)
            {
              site->vents[v]->set_turns(site->vents[v], curr_turns);
              site->vents[v]->time_start = time(NULL);
            }
          }
        }
        else
        {
          printf("Уменьшим обороты curr_turns = %d\n", curr_turns);
          if (curr_turns != 0)
          {
            curr_turns--;
            if (curr_turns < 0)
            {
              curr_turns = 0;
            }
            for (v = 0; v < 2; v++)
            {
              site->vents[v]->set_turns(site->vents[v], curr_turns);
              site->vents[v]->time_start = time(NULL);
            }
          }
        }
      }

      site->temp_in_prev = site->temp_in;
      printf("sub_uvo_th\n");
      sub_uvo_th(site);
    }
    else
    {
      printf("sub_uvo_vent Вентиляторы не включены включим\n");
      site->time_uvo = time(NULL);
      //site->temp_in_prev = site->temp_in; //видимо не сдесь так как тут мы только когда включаем

      if ((site->temp_out > 20) && (site->vents[0]->error == ERROR || site->vents[1]->error == ERROR))
      {
        for (v = 0; v < 2; v++)
        {
          site->vents[v]->set_mode(site->vents[v], 1);
          site->vents[v]->set_turns(site->vents[v], 10);
          site->vents[v]->time_start = time(NULL);
        }
      }
      else
      {
        for (v = 0; v < 2; v++)
        {
          site->vents[v]->set_mode(site->vents[v], 1);
          site->vents[v]->set_turns(site->vents[v], 2);
          site->vents[v]->time_start = time(NULL);
        }
      }

      for (v = 0; v < 2; v++)
      {
        site->vents[v]->time_start = time(NULL);
      }

      sub_uvo_th(site);
    }
  }
  else
  {
    printf("Температура не позваляет работать на вентиляторах\n");
    sub_uvo_pow(site);
  }
}

int sub_uvo_pen(Site* site) {

  printf("Мы в sub_uvo_pen\n");
  int a, v;
  float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"), NULL);
  float temp_heat = strtof(getStr(site->cfg, (void *) "temp_heat"), NULL);

  site->time_uvo = 0;
  site->th->time_start = 0;
  for (v = 0; v < 2; v++)
  {
    site->vents[v]->time_start = 0;
  }

  if (site->temp_in < (temp_support - 2))
  {
    printf("Температура ниже поддержания - 2\n");
    if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1)
    {
      printf("Выключим вентиляторы\n");
      for (v = 0; v < 2; v++)
      {
        site->vents[v]->set_mode(site->vents[v], 0);
        site->vents[v]->set_turns(site->vents[v], 0);
      }
    }

    if ((temp_support - site->temp_out) <= 2)
    {
      site_mode_ac(site);
    }

    if (site->acs[0]->mode == 1 || site->acs[1]->mode == 1)
    {
      printf("Выключим кондиционеры\n");
      for (a = 0; a < 2; a++)
      {
        site->acs[a]->set_mode(site->acs[a], 0);
      }
    }

    if (site->temp_in < temp_heat)
    {
      printf("Переходим в режим догрева\n");
      site_mode_heat(site);
    }
    else
    {
      return EXIT_SUCCESS; //вернем 0, чтоб перейти обратно в УВО
    }
  }

  else
  {
    printf("Температура выше поддержания + 2\n");
    if (site->power == 0)
    {
      //питания нет
      for (a = 0; a < 2; a++)
      {
        site->acs[a]->error = NOPOWER;
      }

      // переходим в режим охлаждения УВО
      // авария на кондиционере
      printf("переходим в режим охлаждения УВО  авария на кондиционере\n");
      site_mode_fail_ac(site);
    }
    else
    {
      //питание есть
      site->penalty++;
      // переходим в режим охлаждения кондиционером
      printf("переходим в режим охлаждения кондиционером\n");
      site_mode_ac(site);
    }
  }
  printf("Выход из sub_uvo_pen\n");
  return EXIT_SUCCESS;
}

void sub_uvo_pow(Site* site) {

  printf("******sub_uvo_pow*******\n");
  int a, v;

  site->time_uvo = 0;
  site->th->time_start = 0;
  for (v = 0; v < 2; v++)
  {
    site->vents[v]->time_start = 0;
  }

  if (site->temp_out < site->temp_in)
  {
    for (v = 0; v < 2; v++)
    {
      site->vents[v]->set_turns(site->vents[v], 255);
    }
  }

  if (site->power == 1)
  {
    site_mode_ac(site);
  }
  else
  {
    for (a = 0; a < 2; a++)
    {
      site->acs[a]->error = NOPOWER;
    }
    site_mode_fail_ac(site);
  }
}

void sub_uvo_th(Site* site) {

  printf("*********sub_uvo_th**************\n");
  int v;
  float temp_dew = strtof(getStr(site->cfg, (void *) "temp_dew"), NULL);
  printf("sub_uvo_th temp_dew = %f site->temp_out = %f \n",temp_dew, site->temp_out);
  if (site->temp_out > temp_dew)
  {
    printf("Где заслонка\n");
    // Заслонка выставлена на улицу
    if (site->th->position != 10)
    {
      printf("Переводим заслонку на улицу\n");
      site->th->set_position(site->th, 10);
      site->th->set_mode(site->th, 1);
    }
  }
  else
  {
    printf("Не настало ли время проверить работает ли заслонка? diff %d\n",(time(NULL) - site->th->time_start));
    if ((time(NULL) - site->th->time_start) >= 30)
    {
      printf("Да настало, а есть ли откуда читать?\n");
      if (site->th_r_exists)
      {
        if (site->th->position == site->th_r)
        {

          int curr_pos = site->th->position;
          if (site->temp_mix >= temp_dew)
          {
            site->th->set_position(site->th, curr_pos++);
          }
          else
          {
            site->th->set_position(site->th, curr_pos--);
          }
        }
        else
        {
          if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1)
          {
            for (v = 0; v < 2; v++)
            {
              //site->vents[v]->set_turns(site->vents[v], 0);
              site->vents[v]->set_mode(site->vents[v], 0);
            }
          }
          site_mode_fail_uvo(site);
        }
      }
      
    }
    else
    {
      printf("Нет пока ненастало\n");
      int curr_pos = site->th->position;
      printf("Температура в миксере %f температура росы %f site->th->position = %d\n",site->temp_mix, temp_dew, site->th->position);
      if (site->temp_mix >= temp_dew)
      {
        printf("Приоткроем заслонку %d\n", curr_pos);
        curr_pos++;
        if(curr_pos > 10) {
          curr_pos = 10;
        }
        site->th->set_position(site->th, curr_pos);
      }
      else
      {
        printf("Призакроем заслонку %d\n", curr_pos);
        curr_pos--;
        if(curr_pos < 0) {
          curr_pos = 0;
        }
        site->th->set_position(site->th, curr_pos);
      }

    }
  }

  site->th->time_start = time(NULL);
  site->temp_in_prev = site->temp_in;

  //возврат в основной цикл

}

int sub_uvo_fail(Site* site) {

  printf("sub_uvo_fail\n");
  float temp_fail = strtof(getStr(site->cfg, (void *) "temp_fail"), NULL);

  if ((site->temp_in) >= temp_fail)
  {
    // прерываем цикл
    // переходим в аварийный режим охлаждения
    site_mode_fail_gen(site);
  }
  else
  {
    // температура нормальная
    float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"), NULL);

    printf("sub_uvo_fail site->temp_in = %f temp_support = %f\n", site->temp_in, temp_support);

    if ((site->temp_in >= (temp_support - 2)) && (site->temp_in <= (temp_support) + 2))
    {
      printf("Температура в пределах site->temp_in >= temp_support - 2 И site->temp_in <= temp_support + 2\n");
      printf("Перейдем sub_uvo_vent\n");
      //sub_uvo_vent(site);
      // передадим исполнение в основную ветку
      return EXIT_FAILURE;
    }
    else
    {
      printf("Температура не в пределах\n");
      printf("Перейдем sub_uvo_pen\n");
      return sub_uvo_pen(site);
    }
  }
}

/* Режим охлаждения кондиционером */
int site_mode_ac(Site* site) {
  //проверил основные моменты
  printf("Режим охлаждения кондиционером!\n");

  write_log(site->logger->eventLOG, "Режим охлаждения кондиционером");

  site->mode = 2;
  site->time_pre = time(NULL);

  int a, v, a_cond;
  for (v = 0; v < 2; v++)
  {
    site->vents[v]->time_start = 0;
  }

  for (a = 0; a < 2; a++)
  {
    site->acs[a]->is_diff = 0;
  }

  int num_ac = atoi(getStr(site->cfg, (void *) "num_ac"));

  int num_ac_tmp = num_ac;

  printf("Количество кондиционеров num_ac_tmp = %d is_diff_0 = %d is_diff_1 = %d\n", num_ac_tmp, site->acs[0]->is_diff,
      site->acs[1]->is_diff);

  if (site->th->position == 10)
  {

    site->th->set_position(site->th, 0);
    site->th->time_start = time(NULL);

  }

  for (a = 0; a < 2; a++)
  {
    site->acs[a]->set_mode(site->acs[a], 1);
    site->acs[a]->time_start = time(NULL);
    printf("Включили кондиционер КОНД_%d время включения %d\n", a, site->acs[a]->time_start);
  }

  printf("До while Режим охлаждения кондиционером\n");
  while (1)
  { // sensors was read - ok

    int ret;
    ret = read_sensors(site);
    if (ret != 0)
    {
      //Ошибка чтения датчиков
      site_mode_fail_ac(site);
    }

    if (difftime(time(NULL), site->time_pre) <= 5)
    { //секунды
      usleep(100000);
      continue;
    }
    else
    {
      printf("*************Принятие решения Режим охлаждения кондиционером***************\n");
      site->time_pre = time(NULL);

      for (a_cond = 0; a_cond < num_ac; a_cond++)
      { //TODO количество кондиционеров брать из конфига
        if (((site->temp_in - site->acs[a_cond]->temp) >= 10) && (site->acs[a_cond]->mode == 1))
        {
          //да КОНД_0 Набрал дельту
          printf("Набрал дельту КОНД_%d %f дельта %f\n", a_cond, site->acs[a_cond]->temp,
              (site->temp_in - site->acs[a_cond]->temp));
          if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1)
          {
            // да
            printf("выключим вентиляцию\n");
            //выключим вентиляцию
            for (v = 0; v < 2; v++)
            {
              if (site->vents[v]->mode == 1)
                site->vents[v]->set_mode(site->vents[v], 0);
            }

            //site->acs[a_cond]->mode == 1;
            //TODO: Проверить по описанию
            //for (a = 0; a < 2; a++) {
            //  if (site->acs[a]->mode == 1)
            //    site->acs[a]->set_mode(site->acs[a], 0);
            //  site->acs[a]->time_start = time(NULL);
            //}

          }
          site->acs[a_cond]->time_start = time(NULL);
          site->acs[a_cond]->is_diff = 1;

        }
        else
        {
          //нет КОНД_x не набрал пока дельту
          printf(" КОНД_%d не набрал пока дельту temp_in %f site->acs[a_cond]->temp %f \n", a_cond, site->temp_in,
              site->acs[a_cond]->temp);
          site->acs[a_cond]->is_diff = 0;

        }

        //600
        // отработан промежуток?
        printf("Цикл КОНД_%d\n", a_cond);
        //printf("600 отработан промежуток? diff %d time_start_%d %d\n",(difftime(time(NULL), site->acs[a_cond]->time_start)), a_cond, site->acs[a_cond]->time_start);
        printf("600 отработан промежуток? time_start_%d %d time %d diff %d %f\n", a_cond, site->acs[a_cond]->time_start,
            time(NULL), time(NULL) - site->acs[a_cond]->time_start, difftime(time(NULL), site->acs[a_cond]->time_start));
        if ((difftime(time(NULL), site->acs[a_cond]->time_start) > 60) //600 для тестов 60
        && (site->acs[a_cond]->mode == 1))
        {
          //дельта набрана?
          printf("ДА 600 прошло дельта набрана? time %d - time_start %d = %f is_diff = %d\n", time(NULL),
              site->acs[a_cond]->time_start, (difftime(time(NULL), site->acs[a_cond]->time_start)),
              site->acs[a_cond]->is_diff);
          if (site->acs[a_cond]->is_diff == 0)
          {
            printf("Авария кондиционера num_ac_tmp = %d\n", num_ac_tmp);
            // Авария кондиционера
            num_ac_tmp--;
            printf("num_ac_tmp = %d", num_ac_tmp);
            // Выключим нерабочий кондиционер
            printf("Выключим нерабочий кондиционер КОНД_%d\n", a_cond);
            if (site->acs[a_cond]->mode == 1)
            {
              site->acs[a_cond]->set_mode(site->acs[a_cond], 0);
              site->acs[a_cond]->mode = 0;
            }
          }

        } //600

      } //for

      printf("еще есть живые кондиционеры? num_ac_tmp = %d\n", num_ac_tmp);
      // еще есть живые кондиционеры?
      if (num_ac_tmp > 0)
      {
        printf("Да еще есть кондиционеры num_ac_tmp = %d\n", num_ac_tmp);
        //да
        float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"), NULL);

        float temp_fail = strtof(getStr(site->cfg, (void *) "temp_fail"), NULL);

        if ((temp_support - site->temp_out) > (site->penalty + 2))
        {
          printf(
              "Температура позволяет перейти на УВО переходим в УВО temp_support %f site->temp_out %f site->penalty %d \n",
              temp_support, site->temp_out, site->penalty);
          // переходим в УВО
          site_mode_uvo(site);

        }
        else
        {
          if (site->temp_in >= temp_fail)
          {
            printf("Температура выше аварийной переходим в аварийный режим site->temp_in %f temp_fail %f\n", site->temp_in,
                temp_fail);
            //переходим в аварийный режим
            site_mode_fail_ac(site);
          }
          else
          {
            printf("работа нормальная\n");
            // работа нормальная
            continue;
          }

        }

      }
      else
      {
        printf("НЕТ живых кондиционеров переходим в аварийный режим\n");
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

  printf("Режим догрева сайта!\n");
  write_log(site->logger->eventLOG, "Режим догрева сайта"); //падает при повторном вызове
  site->mode = 3;

  int a, v;

  site->time_pre = time(NULL);
  site->th_check = 0;

  for (v = 0; v < 2; v++)
  {
    site->vents[v]->time_start = 0;
    site->vents[v]->error = ERROR_HEAT;
  }

  printf("Проверим положение заслонки %d\n", site->th->position);
  if (site->th->position == 10)
  {
    // улица
    site->th->set_position(site->th, 0);
    site->th->time_start = time(NULL);
  }
  //сайт

  int i, th_pos_read;
  if (site->th_r_exists)
  {
    while (i < 300)
    {
      int ret;
      ret = read_sensors(site);
      if (ret != 0)
      {
        //Ошибка чтения датчиков
        site_mode_fail_uvo(site);
      }

      if (((time(NULL) - site->th->time_start) >= 30) && site->th_r_exists)
      {
        if (difftime(time(NULL), site->th->time_start) >= 30)
        {
          printf("Настало время проверить перевилась ли заслонка site->th_check = %d\n", site->th_check);
          //да Необходимо проверить перевилась ли заслонка
          // не было проверки закслонки
          printf("site->th->position = %d th_pos_read = %d", site->th->position, th_pos_read);
          if (site->th->position == th_pos_read)
          {
            printf("да заслонка исправна\n");
            break;
          }
          else
          {
            printf("заслонка не исправна\n");
            site_mode_fail_uvo(site);
          }
        }
        i++;
        sleep(1);
      }
    }
  }

  printf("Перед ЦИКЛ ДОГРЕВ time %d site->time_pre %d diff %f\n", time(NULL), site->time_pre,
      difftime(time(NULL), site->time_pre));
  while (1)
  {
    // читаем датчики
    int ret;
    ret = read_sensors(site);
    if (ret != 0)
    {
      //Ошибка чтения датчиков
      site_mode_fail_uvo(site);
    }

    if (difftime(time(NULL), site->time_pre) <= 2) //30
    { //секунды
      usleep(10000);
      //printf("ЦИКЛ time %d site->time_pre %d diff %f\n", time(NULL), site->time_pre, difftime(time(NULL), site->time_pre));
      continue;
    }
    else
    {
      printf("*****************Принятие решения ДОГРЕВ*******************\n");
      site->time_pre = time(NULL);

      float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"), NULL);

      if (site->temp_in >= temp_support - 4)
      { //TODO необходимо ввести параметр до какой температуры греть
        //да
        printf("Сайт нагрели %f\n выключим ТЭН temp_mix %f", site->temp_in, site->temp_mix);
        set_ten(site, 0);

        if (site->temp_mix > 60)
        {
          printf("ТЭН не остыл еще продолжим вентиляцию\n");
          continue; // продолжаем цикл догрева
        }
        else
        {
          printf("ТЭН остыл выключим вентиляцию\n");
          site->vents[0]->set_turns(site->vents[0], 0); // обороты приточного

          //выключим вентиляцию
          //for (v = 0; v < 2; v++) {
          //  if (site->vents[v]->mode == 1)
          //    site->vents[v]->set_mode(site->vents[v], 0);
          //}
          printf("Перейдем в режим охлаждения УВО\n");
          site_mode_uvo(site); // режим охлаждения уво
        }
      }
      else
      {
        //нет
        printf("Нужно греть сайт\n");
        // TODO: Проверить по описанию
        if (site->vents[0]->mode == 0)
        {
          site->vents[0]->set_mode(site->vents[0], 1);
          site->vents[0]->set_turns(site->vents[0], 10);
          site->vents[0]->time_start = time(NULL);
        }

        if (difftime(time(NULL), site->vents[0]) > 30) //TODO: 30
        {
          printf("Проверим вращается ли вентилятор\n");
          //да

          if ((site->vents[0]->turns == site->tacho1) || (site->tacho1_exists != 1))
          {
            printf("Да вращается\n");
            //да
            site->vents[0]->time_start = time(NULL);
            if ((site->ten == 1) && site->vents[0]->error == ERROR_HEAT)
            {
              printf("Пока неизвестно вращается ли вентилятор\n");
              //да
              continue;

            }
            else
            {
              //нет
              printf("Да вращается включим ТЭН\n");
              set_ten(site, 1);
              continue;
            }
          }
          else
          {
            printf("АВАРИЯ вентилятора Авария вентиляции - охлаждение кондиционером\n");
            //нет
            set_ten(site,0);
            site->vents[0]->set_turns(site->vents[0], 0);
            site->vents[0]->error = ERROR;

            site_mode_ac(site); // Авария вентиляции - охлаждение кондиционером
          }
        }
        else
        {
          printf("Время проверки работает ли вентилятор еще не настало\n");
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
  printf("Режим авария УВО!\n");
  write_log(site->logger->eventLOG, "Режим авария УВО");
  site->mode = 4;
  site->time_pre = time(NULL);

  int a, v;
  for (v = 0; v < 2; v++)
  {
    site->vents[v]->time_start = 0;
  }

  for (a = 0; a < 2; a++)
  {
    site->acs[a]->is_diff = 0;
  }

  int num_ac = atoi(getStr(site->cfg, (void *) "num_ac"));

  int num_ac_tmp = num_ac;

  while (1)
  { // sensors was read - ok

    int ret;
    ret = read_sensors(site);
    if (ret != 0)
    {
//Ошибка чтения датчиков
      continue;
    }

    if (difftime(time(NULL), site->time_pre) <= 30)
    { //секунды
      continue;
    }
    else
    {

      site->time_pre = time(NULL);

      for (a = 0; a < 2; a++)
      {
        if ((site->temp_in >= 10) && (site->acs[a]->mode == 1))
        {
          //да
          if (site->vents[0]->mode == 1 || site->vents[1]->mode == 1)
          {
//да
// выключим вентиляцию
            for (v = 0; v < 2; v++)
            {
              site->vents[v]->set_mode(site->vents[v], 0);
            }
// TODO: Проверить по описанию
            for (a = 0; a < 2; a++)
            {
              site->acs[a]->set_mode(site->acs[a], 1);
              site->acs[a]->time_start = time(NULL);
            }
          }
        }

        //600
        // отработан промежуток?
        if ((difftime(time(NULL), site->acs[a]->time_start) > 600) && (site->acs[a]->mode == 1))
        {
          //дельта набрана?
          if (site->acs[a]->is_diff == 0)
          {
// Авария кондиционера
            num_ac_tmp--;
            site->acs[a]->set_mode(site->acs[a], 0);
          }
        }
      }

// Eще есть живые кондиционеры?
      if (num_ac_tmp > 0)
      {
        //да
        float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"),
        NULL);

        if ((temp_support - site->temp_out) >= (site->penalty + 2))
        {
          if (site->temp_in <= temp_support - 2)
          {
// переходим в УВО
            site_mode_uvo(site);
          }
          else
          {
//продолжаем в текущем режиме
            continue;
          }
        }
        else
        {
          //продолжаем в текущем режиме
          continue;
        }

      }
      else
      {
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
  printf("Режим авария кондиционеров!\n");
  write_log(site->logger->eventLOG, "Режим авария кондиционеров");
  site->mode = 5;

  site->time_pre = time(NULL);
  site->th_check = 0;
  site->th->time_start = 0;

  int a, v;

  if (site->th->position != 10)
  {
    site->th->set_position(site->th, 10);
    site->th->time_start = time(NULL);
  }

  while (1)
  {

    int th_pos_read;
    int vents_r[2];

    int ret;
    ret = read_sensors(site);
    if (ret != 0)
    {
//Ошибка чтения датчиков
      for (v = 0; v < 2; v++)
      {
        site->vents[v]->set_mode(site->vents[v], 1);
        site->vents[v]->set_turns(site->vents[v], 10); // 100%
        site->vents[v]->time_start = time(NULL);
      }

      continue;

    }

    if (difftime(time(NULL), site->time_pre) <= 30)
    { //секунды
      continue;
    }
    else
    {
      if ((difftime(time(NULL), site->th->time_start) >= 30) && (site->th_check == 0))

        if (site->th->position != th_pos_read)
        {
          for (v = 0; v < 2; v++)
          {
            site->vents[v]->set_mode(site->vents[v], 1);
            site->vents[v]->set_turns(site->vents[v], 10); // 100%
            site->vents[v]->time_start = time(NULL);
            site_mode_uvo(site); // перейдем в режим УВО
          }

        }
        else
        {
          site->th_check = 1;
          continue;
        }
    }

    site->time_pre = time(NULL);

// выключим все кондиционеры
    for (a = 0; a < 2; a++)
    {
      site->acs[a]->set_mode(site->acs[a], 0);
    }

// включим вентиляцию
    for (v = 0; v < 2; v++)
    {
      if (site->vents[v]->mode == 0)
      {
        site->vents[v]->set_turns(site->vents[v], 10); // 100%
        site->vents[v]->time_start = time(NULL);
      }
    }

//работаем с вытяжным
    if ((difftime(time(NULL), site->vents[0]->time_start) > 30))
    {
      if (site->vents[0]->turns == vents_r[0])
      {
        site->vents[0]->error = NOERROR;
      }
      else
      {
        // Заглушка, можем уйти в бесконечный цикл
        // site_mode_ac(site);
      }
    }

// проверяем кондиционеры
//
    for (a = 0; a < 2; a++)
    {
      if (site->acs[a]->error == NOPOWER)
      {
        if (site->power == 1)
          site_mode_ac(site); // переходим в нормальный режим;
      }
    }

    float temp_support = strtof(getStr(site->cfg, (void *) "temp_support"),
    NULL);

    if (site->temp_in > temp_support - 2)
    {
//Переходим на уво
      site_mode_uvo(site);
    }

// выключим вениляцию
    for (v = 0; v < 2; v++)
    {
      if (site->vents[v]->mode == 1)
      {
        site->vents[v]->set_mode(site->vents[v], 0);
        site->vents[v]->set_turns(site->vents[v], 0);
        site->vents[v]->time_stop = time(NULL);
      }
    }

//выключим кондиционеры
    for (a = 0; a < 2; a++)
    {
      site->acs[a]->set_mode(site->acs[a], 0);
      site->acs[a]->time_stop = time(NULL);
    }

    float temp_heat = strtof(getStr(site->cfg, (void *) "temp_heat"), NULL);

    if (site->temp_in < temp_heat)
    {
      site_mode_heat(site);
    }
    else
    {
      continue;
    }

  }
  return 1;
}

/* Превышена температура аварии - Аварийный режим охлаждения*/
int site_mode_fail_gen(Site* site) {

  printf("Общий аварийный режим!\n");
//write_log(site->logger->eventLOG, "Общий аварийный режим");
  site->mode = 6;

  if (site->temp_in - site->temp_out)
  {
//да
// Охлаждаем УВО
    site_mode_uvo(site);

  }
  else
  {
//нет
// Охлаждаем кондиционерами
    site_mode_ac(site);

  }

  return 1;
}

ConfigTable* read_config(char* filepath) {
  ConfigTable* cfg = readConfig(filepath);
  return cfg;
}

int set_mode(Site* site, int val) {

  return 0;

  if (val >= 0 && val <= 6)
  { // acceptable numbers
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

int set_ten(Site* site, int val) {

  i2cOpen();

  int addr, value, bit = 0;
  addr = strtol(getStr(site->cfg, "a_relay"), NULL, 16);
  printf("Изменим тен\n");
  char buf[1];

  if (ioctl(g_i2cFile, I2C_SLAVE, addr) < 0)
  {
    printf("Failed to acquire bus access and/or talk to slave.\n");
  }

  if (read(g_i2cFile, buf, 1) != 1)
  {
    printf("Error reading from i2c\n");
  }

  value = (int) buf[0];

  if ((val == 1) || (val == 0))
  {
    if (val == 1)
      value &= ~(1 << bit); // очистим бит
    else
      value |= (1 << bit); // установим бит

    printf("Управляем регистром, адрес %x, значение %d, %x , бит %d \n", addr, val, value, bit);
    set_i2c_register(g_i2cFile, addr, value, value);
    site->ten = val;
    i2cClose();
    return 1;
  }
  else
  {
    // wrong value
    // неправильное значение
    return 0;
  }
  return 0;
}

// Запишем текущее состояние параметров
int write_current() {
  return 0;
}

//TODO: Переписать в виде синглетона
//TODO: Добавить мьютексы для многопоточного доступа к переменным
Site* site_new(char* filename) {

  Site* site = malloc(sizeof(Site));
//initialize hardware
  int i;
  for (i = 0; i < 2; i++)
  {
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

  site->cfg = read_config(filename);

  site->set_mode = set_mode;
  site->set_ten = set_ten;
//site->get_ac_time_work = get_time_work;
// printf("Попытаемся создать журнал\n");
  site->logger = create_logger();

  return site;
}
