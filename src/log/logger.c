
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"
#include "file_logger.h"
#include "net_logger.h"
#include "../hw/site.h"

Logger* create_logger() {

  //printf("Создадим необходимые журналы\n");
  Logger* log = malloc(sizeof(Logger));
  log->dataLOG = create_filelog("data.log"); //TODO: считать путь из конфига
  //log->dataLOG_n = create_netlog("", 80); //TODO: считать параметры из конфига

  log->eventLOG = create_filelog("event.log"); //TODO: считать путь из конфига
  //log->eventLOG_n = create_netlog("", 80); //TODO: считать параметры из конфига

  return log;
}

void logger_free(Logger* logger) {
  //close_filelog(logger->dataLOG);
  //close_filelog(logger->eventLOG);
  free(logger->dataLOG);
  free(logger->eventLOG);
}

void run_logger(Site* site){
  printf("Запустим цикл логгера\n");
  while(1){
    printf("Цикл работы логгера\n");
    write_data_log(site);
    sleep(30);
  }

void test_logger(){

  write_dl("This is test of arbitrary write in data log" , 2);

}

