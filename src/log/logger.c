#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"
#include "file_logger.h"
#include "net_logger.h"

Logger* create_logger() {

  Logger* log = malloc(sizeof(Logger));
  log->dataLOG = create_filelog("data.log"); //TODO: считать из конфига
  log->dataLOG_n = create_netlog("", 80); //TODO: считать параметры из конфига

  log->eventLOG = create_filelog("event.log");
  log->eventLOG_n = create_netlog("", 80);

  return log;

}

void logger_free() {

  //TODO: очистим ресурсы памяти
}

int dump_full_log(char *name) {

  return 0;

}

int dump_fail_log(char *name) {

  return 0;

}

int dump_data_log(char *name) {

  return 0;

}

int send_full_log(char *name) {

  return 0;

}

int sent_fail_log(char *name) {

  return 0;

}

int sent_data_log(char *name) {

  return 0;

}
