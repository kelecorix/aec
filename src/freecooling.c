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

int main(int argc, char *argv[]) {

  pthread_t threadL, threadU, threadC;
  int retL, retU;
  void *ret;
  char *filename = "freecooling.conf";

  Site* site = site_new(filename);

  if (site->cfg)
    printf("Config was read ok!\n");

  site->conn = create_server_conn("127.0.0.1:5001");
  if (site->conn == 0)
    printf("OWFS connection established!\n");
  else
    printf("OWFS connection not fins. Fire up OWFS server!");

  read_sensors(site);

  sleep(4);

  //Workers Воркеры - выполняют параллельно свои операции

//    if (pthread_create(&threadL, NULL, run, (void*) site)) {
//      fprintf(stderr, "Error creating algo thread\n");
//      return 1;
//    }

//    if (pthread_create(&threadU, NULL, run_ui, (void*) site)) {
//      fprintf(stderr, "Error creating UI thread\n");
//      return 1;
//    }

// ждем пока потоками завершаться
// по идде сюда не должно дойти
//pthread_join(threadL, NULL);
  //pthread_join(threadU, NULL);

  return EXIT_SUCCESS;

}
