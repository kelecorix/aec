#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"

FileLogWriter* create_filelog(char* filename) {

  printf("Создадим новый журнал, %s\n", filename);
  FileLogWriter* log = malloc(sizeof(FileLogWriter));

  log->filename = filename;
  log->fp = fopen(filename, "a");
  if (!log->fp) {
    fprintf(stderr, "could not open log file %s", filename);
    return NULL;
  }

  return log;
}

int write_log(FileLogWriter* flw, char* message) {

  printf("Запишем данные в журнал %s\n", flw->filename);
  time_t timer;
  struct tm* tm_info;
  char date[50];

  timer = time(NULL);
  printf("Получили текущее время\n");
  tm_info = localtime(&timer);
  printf("Получиили локальное время\n");

  strftime(date, 25, "%Y:%m:%d %H:%M:%S", tm_info);
  printf("Дата %s\n", date);
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
