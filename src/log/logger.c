
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"
#include "file_logger.h"
#include "net_logger.h"
#include "../hw/site.h"

/*
 *
 *
 *
 */
Logger* create_logger() {

  Logger* log = malloc(sizeof(Logger));
  log->dataLOG = create_filelog(concat(gcfg->cdir, "/data.log"));
  //log->dataLOG_n = create_netlog("", 80);

  log->eventLOG = create_filelog(concat(gcfg->cdir, "/event.log"));
  //log->eventLOG_n = create_netlog("", 80);

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

  logD(site->logger->dataLOG, 0, "this is test for %s number %d", site->logger->dataLOG->filename, 12);

}

