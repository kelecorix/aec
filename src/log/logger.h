#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "file_logger.h"
#include "net_logger.h"


/** The log writer object */
typedef struct LogWriter {


} LogWriter;

typedef struct Logger {
  int level;
  FileLogWriter dataLOG;   // журналирование данных
  FileLogWriter eventLOG;  // журналирование событий
  NetLogWriter dataLOG_n;  // удаленное журналирование данны
  NetLogWriter eventLOG_n; // удаленное журанлирование собыйти
} Logger;

#endif /*LOG_H_*/
