#ifndef FILE_LOGGER_H_
#define FILE_LOGGER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"

typedef struct FileLogWriter {
  LogWriter base;
  FILE *fp;
} FileLogWriter;

FileLogWriter* create_filelog(char* filename);
int close_filelog(FileLogWriter* flog);

#endif /* FILE_LOGGER_H_ */
