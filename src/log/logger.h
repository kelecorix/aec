#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "file_logger.h"
#include "net_logger.h"

typedef struct Logger {
  int level;
  FileLogWriter* dataLOG;   // журналирование данных
  FileLogWriter* eventLOG;  // журналирование событий
  NetLogWriter* dataLOG_n;  // удаленное журналирование данны
  NetLogWriter* eventLOG_n; // удаленное журанлирование событий
} Logger;

void run_logger();
void test_logger();

#endif /*LOG_H_*/
