#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"

FileLogWriter* create_filelog(char* filename) {

  FileLogWriter* log = malloc(sizeof(FileLogWriter));

  FILE *fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "could not open log file %s", filename);
    return NULL;
  }

  log->fp = fp;

  return log;

}

int close_filelog(FileLogWriter* flog){

  fclose(flog->fp);
  return 0;

}
