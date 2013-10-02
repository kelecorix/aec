#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#include "net_logger.h"
#include "../hw/site.h"
#include "../utils/utils.h"
#include "logger.h"
#include "file_logger.h"


/*
 *
 *
 *
 */
Logger* create_logger() {

  char* filepath;
  char* dname="data.log";
  char* ename="event.log";

  Logger* log = malloc(sizeof(Logger));
  filepath = concat(gcfg->ldir, dname);
  log->dataLOG = create_filelog(filepath);

  filepath = concat(gcfg->ldir, ename);
  log->eventLOG = create_filelog(filepath);

  return log;
}

/*
 *
 *
 *
 */
void logger_free(Logger* logger) {
  free(logger->dataLOG);
  free(logger->eventLOG);
}

/*
 *
 *
 *
 */
void run_logger(Site* site){
  printf("Запустим цикл логгера\n");
  while(1){
    printf("Цикл работы логгера\n");
    write_data_log(site);
    sleep(10);
  }
}

/*
 *
 *
 *
 */
void test_logger(){

  logD(gcfg->logger->dataLOG, 0, "this is test for %s number %d", gcfg->logger->dataLOG->filepath, 12);

}

