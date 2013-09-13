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

Site* site;

int main(int argc, char *argv[]) {

  pthread_t threadA, threadU, threadL;
  int retA, retU, retL;
  void *ret;
  char *filename = "freecooling.conf";

  site = site_new(filename);

  if (site->cfg)
    printf("Config was read ok!\n");

  site->conn = create_server_conn("127.0.0.1:4304");
  if (site->conn == 0)
    printf("OWFS connection established!\n");
  else
    printf("OWFS connection not fins. Fire up OWFS server!\n");

  sleep(4);

  //Workers Воркеры - выполняют параллельно свои операции

  if (pthread_create(&threadA, NULL, run, (void*) site)) {
    fprintf(stderr, "Error creating algo thread\n");
    printf("Error creating algo thread\n");
    return 1;
  }

  sleep(4);

  if (pthread_create(&threadU, NULL, run_ui, (void*) site)) {
    fprintf(stderr, "Error creating UI thread\n");
    printf("Error creating UI thread\n");
    return 1;
  }

  sleep(4);

  printf("Создадим поток журналирования\n");
  if(pthread_create(&threadL, NULL, run_logger, (void*) site)) {
    fprintf(stderr, "Error creating Logger thread\n");
    printf("Error creating Logger thread\n");
    return 1;
  }

  sleep(4);

// ждем пока потоками завершаться
// по идде сюда не должно дойти
  pthread_join(threadA, retA);
  pthread_join(threadU, retU);
  //pthread_join(threadL, retL);

  // Для тестов оборудования
  //i2cTestHardware();

  return EXIT_SUCCESS;

}
