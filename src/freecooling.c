#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "config/config.h"
#include "hw/site.h"
#include "hw/ac.h"
#include "hw/throttle.h"
#include "hw/vent.h"
#include "hw/ow.h"
#include "ui/lcd.h"
#include "ui/keyboard.h"
#include "ui/ui.h"
#include "hw/i2c.h"
#include "log/logger.h"
#include "utils/utils.h"

Site* site;
Cfg*  gcfg;
int   gmode;

/*
 *
 *
 *
 */
void process_args(int argc, char *argv[]) {

  int i, val;
  int dFlag = 0;  // debug mode
  int pFlag = 0;  // printfflag
  int thFlag = 0; // test hardware
  int saFlag = 0; // owfs server adress
  int cfFlag = 0; // directory to store files
  int lfFlag = 0; // directory to store log
  int mpFlag = 0; // moint point for owfs
  int tlFlag = 0; // time log period
  int tmFlag = 0; // moto log period
  int hFlag  = 0;    // help

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      dFlag = 1;
      if (i + 1 <= argc - 1) {
        i++;
        val = atoi(argv[i]);
        if ((val <= 4) && (val > 0)) {
          gcfg->debug = val;
        } else {
          fprintf(stderr, "Должен быть аргумент, например: -d 2. Используйте -h для подсказки\n");
          exit(1);
        }
      } else {
        fprintf(stderr, "Должен быть аргумент, например: -d 2. Используйте -h для подсказки\n");
        exit(1);
      }
    }

    if (strcmp(argv[i], "-p") == 0) {
      pFlag = 1;
      if (i + 1 <= argc - 1) {
        i++;
        val = atoi(argv[i]);
        if ((val <= 1) && (val >= 0)) {
          gcfg->gpf = val;
        } else {
          fprintf(stderr, "Должен быть аргумент, например: -p 1. Используйте -h для подсказки\n");
          exit(1);
        }
      } else {
        fprintf(stderr, "Должен быть аргумент, например: -p 1. Используйте -h для подсказки\n");
        exit(1);
      }
    }

    if (strcmp(argv[i], "-th") == 0) {
      thFlag = 1;
      gmode = 2;
    }

    if (strcmp(argv[i], "-sa") == 0) {
      //saFlag = 1;
      if (i + 1 <= argc-1) {
        i++;
        gcfg->saddr = argv[i];
      } else {
        fprintf(stderr, "Неправильный аргумент, например: -sa 127.0.0.1:3001. Используйте -h для подсказки\n");
        exit(1);
      }
    }
    if (strcmp(argv[i], "-cf") == 0) {
      cfFlag = 1;
      if (i + 1 <= argc-1) {
        i++;
        gcfg->cdir = argv[i];
      } else {
        fprintf(stderr, "Неправильный аргумент, например: -cf /etc/conf/. Используйте -h для подсказки\n");
        exit(1);
      }
    }

    if (strcmp(argv[i], "-lf") == 0) {
      lfFlag = 1;
      if (i + 1 <= argc-1) {
        i++;
        gcfg->ldir = argv[i];
      } else {
        fprintf(stderr, "Неправильный аргумент, например: -lf /etc/log/. Используйте -h для подсказки\n");
        exit(1);
      }
    }
    if (strcmp(argv[i], "-mp") == 0) {
      mpFlag = 1;

      if (i + 1 <= argc-1) {
        i++;
        gcfg->mpoint = argv[i];
      } else {
        fprintf(stderr, "Неправильный аргумент, например: -mp  /mnt/1wire/. Используйте -h для подсказки\n");
        exit(1);
      }
    }

    if (strcmp(argv[i], "-tl") == 0) {
      tlFlag = 1;
      if (i + 1 <= argc-1) {
        i++;
        val = atoi(argv[i]);
        if (val > 0) {
          gcfg->ltime = val;
        } else {
          fprintf(stderr, "Должен быть аргумент, например: -tl 25. Используйте -h для подсказки\n");
          exit(1);
        }
      } else {
        fprintf(stderr, "Должен быть аргумент, например: -tl 25. Используйте -h для подсказки\n");
        exit(1);
      }
    }

    if (strcmp(argv[i], "-tm") == 0) {
      //tmFlag = 1;
      if (i + 1 <= argc-1) {
        i++;
        val = atoi(argv[i]);
        if (val > 0) {
          gcfg->mtime = val;
        } else {
          fprintf(stderr, "Должен быть аргумент, например: -tm 30. Используйте -h для подсказки\n");
          exit(1);
        }
      } else {
        fprintf(stderr, "Должен быть аргумент, например: -tm 30. Используйте -h для подсказки\n");
        exit(1);
      }
    }

    if (strcmp(argv[i], "-h") == 0) {
      hFlag = 1;
      printf("\n");
      printf("Freecooling системное приложение, предназначенное для обеспечения единого температурного режима\n");
      printf("в помещении базовой станции. Система имеет несколько режимов работы, предназначенных для различных\n");
      printf("потребностей. По умолчанию, программа не требует аргументов и запускается в штатном режиме, выполняя\n");
      printf("установленный алгорим в соответствии с параметрами окружающей среды, поступающих с датчиков.\n");

      printf("\n");
      printf("Ключ: -d\n");
      printf("\tПредназначен для запуска системы в режиме отладки, необходимо использовать ключ -d\n");
      printf("\tсо значением опции от 1 до 4. \n");
      printf("\tЗначения 1..4 указывают программе, уровень вывода сообщений, на уровне 1 будут \n");
      printf("\tвыводится самые обощенные данные.\n");
      printf("\tНа уровне 4 самые низкоуровневые, связанные с работой конкретных устройств.\n");
      printf("\tНапример: freecoolinf -d 3\n");
      printf("\n");

      printf("Ключ: -p\n");
      printf("\tПредназначен для вывод сообщений в терминал, необходимо использовать значение 0 или 1.\n");
      printf("\tЕсли ключ имеет значени 1, тогда сообщения лога будут выводится в терминал.\n");
      printf("\tНапример: freecooling -d 4 -p 1\n");

      printf("\n");
      printf("Ключ: -sa\n");
      printf("\tПредназначен для указания программе адреса сервера owfs или изменения порта.\n");
      printf("\tПо умолчанию, 127.0.0.1:3001\n");
      printf("\tНапример: freecooling -sa 192.168.1.1:8001\n");

      printf("\n");
      printf("Ключ: -mp\n");
      printf("\tПредназначен для указания точки моентирования owfs сервера,\n");
      printf("\tиз которого можно производить считывание датчиков.\n");
      printf("\tПо умолчанию, /bus.0/ \n");
      printf("\tНапример: freecooling -mp /mnt/1wire/ \n");

      printf("\n");
      printf("Ключ: -cf\n");
      printf("\tПредназначен для указания директории конфигурационного файла программы.\n");
      printf("\tПо умолчаению, совпадает с расположением исполняемого файла программы.\n");
      printf("\tНапример: freecooling -cf /etc/conf/ \n");

      printf("\n");
      printf("Ключ: -lf\n");
      printf("\tПредназначен для указания директории записи логов.\n");
      printf("\tПо умолчанию, используется стандартное расположение /var/log/ .\n");
      printf("\tНапример: freecooling -lf /etc/log/ \n");

      printf("\n");
      printf("Ключ: -th\n");
      printf("\tПредназначен для тестирования связности оборудования.\n");
      printf("\tЕсли используется этот флаг, все остальные игнорируются.\n");
      printf("\tНапример: freecooling -th \n");

      printf("\n");
      printf("Ключ: -tl\n");
      printf("\tПредназначен для указания периодичности записи глобального лога, в минутах.\n");
      printf("\tНапример: freecooling -tl 22 \n");

      printf("\n");
      printf("Ключ: -tm\n");
      printf("\tПредназначен для указания периодичности записи наработки моточасов оборудования\n");
      printf("\tВ минутах.По умолчанию: 0, без записи.");
      printf("\tНапример: freecooling -tm 30 \n");

      printf("\n");
      printf("Ключ: -h\n");
      printf("\tПредназначен для вывода описания работы с программой.\n");
      exit(1);
    }
  }

  // заготовки для специфичной обработки флагов
  if (dFlag == 0)  dFlag = 0;
  if (pFlag == 0)  pFlag = 0;
  if (thFlag == 0) thFlag =0;
  if (saFlag == 0) saFlag=0;
  if (cfFlag == 0) cfFlag=0;
  if (lfFlag == 0) lfFlag=0;
  if (mpFlag == 0) mpFlag=0;
  if (tlFlag == 0) tlFlag=0;
  if (tmFlag == 0) tmFlag=0;
  if (hFlag ==0)   hFlag=0;

  // Разбираемся с несовместимыми флагами

}

int main(int argc, char *argv[]) {

  int opts = 0; //0 - no need to process cli options, 1 - process options
  gmode = 0;
  if (argc == 1)
    opts = 0;
  else
    opts = 1;
  printf("считали опции");
  gcfg = new_gcfg();

  if (opts)
    process_args(argc, argv);
  printf("обработали опции");
  gcfg->logger = create_logger();

  site = site_new();
  pop_hw();
  printf("создали оборудование");
  if (site->cfg)
    log_2("Config was read ok!\n");

  gcfg->conn = create_server_conn(gcfg->saddr);

  if (gcfg->conn == 0)
    log_4("OWFS connection established!\n");
  else
    log_4("OWFS connection not fins. Fire up OWFS server!\n");

  sleep(4);

  // Выполняем тестирование оборудования
  if (gmode == 2) {
    i2cTestHardware();
    exit(1);
  }

  // Запустим программу в основном режиме
  pthread_t threadA, threadU, threadL, threadM;
  void* retA = NULL;
  void* retU = NULL;
  void* retL = NULL;
  void* retM = NULL;

  if (pthread_create(&threadA, NULL, (void *) run, (void*) site)) {
    log_4("Error creating algo thread\n");
    return 1;
  }

  sleep(4);

  if (pthread_create(&threadU, NULL, (void *) run_ui, (void*) site)) {
    log_4("Error creating UI thread\n");
    return 1;
  }

  sleep(4);

  if (pthread_create(&threadL, NULL, (void *) run_logger, (void*) site)) {
    log_4("Error creating Logger thread\n");
    return 1;
  }

  sleep(4);

  // поток записи моточасов
  if (gcfg->mtime > 0) {
    if (pthread_create(&threadM, NULL, (void *) run_moto, (void*) site)) {
      log_4("Error creating moto logger thread\n");
      return 1;
    }
  }

  pthread_join(threadA, retA);
  pthread_join(threadU, retU);
  pthread_join(threadL, retL);
  pthread_join(threadM, retM);

  return EXIT_SUCCESS;
}

