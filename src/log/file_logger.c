
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"

FileLogWriter* create_filelog(char* filename) {

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

  time_t timer;
  struct tm* tm_info;
  char date[50];
  int event_t; // event type, тип события

  timer = time(NULL);
  tm_info = localtime(&timer);

  strftime(date, 25, "%Y:%m:%d %H:%M:%S", tm_info);
  fprintf(flw->fp, date);
  fprintf(flw->fp, "|");
  fprintf(flw->fp, message);
  fprintf(flw->fp, "\n");
  fclose(flw->fp);

  return 0;
}

void write_data_log(){

  FILE *fp = site->logger->dataLOG->fp;

  time_t timer;
  struct tm* tm_info;
  char date[50];
  int event_t;           // event type, тип события

  timer = time(NULL);
  tm_info = localtime(&timer);

  strftime(date, 25, "%Y:%m:%d %H:%M:%S", tm_info);
  fprintf(fp, date);
  fprintf(fp, "|");
  fprintf(fp, itoa(event_t, NULL, 10));
  fprintf(fp, "|");
  fprintf(fp, itoa(site->temp_in, NULL, 10));
  fprintf(fp, ";");
  fprintf(fp, itoa(site->temp_out, NULL, 10));
  fprintf(fp, ";");
  fprintf(fp, itoa(site->temp_mix, NULL, 10));
  fprintf(fp, ";");
  fprintf(fp, itoa(site->temp_evapor1, NULL, 10));
  fprintf(fp, ";");
  fprintf(fp, itoa(site->temp_evapor2, NULL, 10));

}

int close_filelog(FileLogWriter* flog) {
  fclose(flog->fp);
  return 0;
}
