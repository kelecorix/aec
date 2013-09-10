#ifndef FILE_LOGGER_H_
#define FILE_LOGGER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"

typedef struct FileLogWriter {
  FILE *fp;
  char* filename;
} FileLogWriter;

FileLogWriter* create_filelog(char* filename);
int close_filelog(FileLogWriter* flog);
int write_log(FileLogWriter* flw, char* message);

#endif /* FILE_LOGGER_H_ */
