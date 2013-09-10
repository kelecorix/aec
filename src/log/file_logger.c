#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"

FileLogWriter* create_filelog(char* filename) {

  FileLogWriter* log = malloc(sizeof(FileLogWriter));

  FILE *fp;
  log->filename = filename;
  log->fp = fp;

  return log;
}

int write_log(FileLogWriter* flw, char* message) {

  flw->fp = fopen(flw->filename, "a");
  if (!flw->fp) {
    fprintf(stderr, "could not open log file %s", flw->filename);
    return NULL;
  }

  time_t timer;
  struct tm* tm_info;
  char *date;

  time(&timer);
  tm_info = localtime(&timer);

  strftime(date, 25, "%Y:%m:%d %H:%M:%S", tm_info);
  fprintf(flw->fp, date);

  fprintf(flw->fp, " | ");
  fprintf(flw->fp, message);
  fprintf(flw->fp, "\n");
  fclose(flw->fp);

  return 0;
}

int close_filelog(FileLogWriter* flog) {

  fclose(flog->fp);
  return 0;

}
