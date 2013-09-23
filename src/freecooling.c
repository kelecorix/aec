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
#include "ui/buttons.h"
#include "ui/keyboard.h"
#include "ui/ui.h"
#include "hw/i2c.h"
#include "log/logger.h"

Site* site;

int main(int argc, char *argv[]) {


  if (argc < 2)
  {
      fprintf(stderr, "usage: %s debug_level printf_flaf \n", argv[0]);
      exit(1);
  }

  pthread_t threadA, threadU, threadL;
  int retA, retU, retL;
  void *ret;
  char *filename = "freecooling.conf";

  site = site_new(filename);

  site->debug = atoi(argv[1]);
  site->gpf = atoi(argv[2]);

  printf("%d %d", site->debug, site->gpf);

  test_logger();

  if (site->cfg)
    log4("Config was read ok!\n");

  site->conn = create_server_conn("127.0.0.1:3001");

  if (site->conn == 0)
    log4("OWFS connection established!\n");
  else
    log4("OWFS connection not fins. Fire up OWFS server!\n");

  sleep(4);

  //Workers Воркеры - выполняют параллельно свои операции

  if (pthread_create(&threadA, NULL, run, (void*) site)) {
    fprintf(stderr, "Error creating algo thread\n");
    log4("Error creating algo thread\n");
    return 1;
  }

  sleep(4);

  if (pthread_create(&threadU, NULL, run_ui, (void*) site)) {
    fprintf(stderr, "Error creating UI thread\n");
    log4("Error creating UI thread\n");
    return 1;
  }

  sleep(4);

  printf("Создадим поток журналирования\n");
  if(pthread_create(&threadL, NULL, run_logger, (void*) site)) {
    fprintf(stderr, "Error creating Logger thread\n");
    log4("Error creating Logger thread\n");
    return 1;
  }

  sleep(4);

// ждем пока потоками завершаться
// по идде сюда не должно дойти
  pthread_join(threadA, retA);
  pthread_join(threadU, retU);
  pthread_join(threadL, retL);

  // Для тестов оборудования
  //i2cTestHardware();

  return EXIT_SUCCESS;

}
