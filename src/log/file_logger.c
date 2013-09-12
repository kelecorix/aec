#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"

FileLogWriter* create_filelog(char* filename) {

  printf("Создадим новый журнал, %s", filename);
  FileLogWriter* log = malloc(sizeof(FileLogWriter));

  FILE *fp;
  log->filename = filename;
  fp = fopen(filename, "a");
  if (!fp) {
    fprintf(stderr, "could not open log file %s", filename);
    return NULL;
  }
  log->fp = fp;

  return log;
}

int write_log(FileLogWriter* flw, char* message) {

  time_t timer;
  struct tm* tm_info;
  char *date;

  time(&timer);
  tm_info = localtime(&timer);

  strftime(date, 25, "%Y:%m:%d %H:%M:%S", tm_info);
  fprintf(flw->fp, date);

//  strcpy(str_send_log, tmp_time);
//  strcat(str_send_log,"|1|");
//  strcat(str_send_log,tmp_temp_sayt);
//  strcat(str_send_log,tmp_temp_ulica);
//  strcat(str_send_log,tmp_temp_miks);
//  strcat(str_send_log,tmp_temp_kond0);
//  strcat(str_send_log,tmp_temp_kond1);

  fprintf(flw->fp, " | ");
  fprintf(flw->fp, message);
  fprintf(flw->fp, "\n");
  //fclose(flw->fp);

  return 0;
}

int close_filelog(FileLogWriter* flog) {

  fclose(flog->fp);
  return 0;

}
