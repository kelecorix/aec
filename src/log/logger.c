#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"
#include "file_logger.h"
#include "net_logger.h"

Logger* create_logger() {

  Logger* log = malloc(sizeof(Logger));
  log->dataLOG = create_filelog("data.log"); //TODO: считать путь из конфига
  log->dataLOG_n = create_netlog("", 80); //TODO: считать параметры из конфига

  log->eventLOG = create_filelog("event.log"); //TODO: считать путь из конфига
  log->eventLOG_n = create_netlog("", 80); //TODO: считать параметры из конфига

  return log;

}

void logger_free() {

  //TODO: очистим ресурсы памяти
}

