#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"
#include "file_logger.h"
#include "net_logger.h"

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

  close_filelog(logger->dataLOG);
  close_filelog(logger->eventLOG);
  free(logger->dataLOG);
  free(logger->eventLOG);
}

void run_logger(){

}
