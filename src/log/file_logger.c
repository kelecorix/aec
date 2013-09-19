#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"
#include "../hw/site.h"
#include "../hw/vent.h"

FileLogWriter* create_filelog(char* filename) {

  FileLogWriter* log = malloc(sizeof(FileLogWriter));
  log->filename = filename;

  return log;
}

void write_log(FileLogWriter* flw, char* message) {

  FILE* fp = flw->fp;
  char *filename = flw->filename;
  fp = fopen(filename, "a");
  if (!fp) {
    fprintf(stderr, "could not open log file %s", filename);
  }

  time_t timer;
  struct tm* tm_info;
  char date[50];

  timer = time(NULL);
  tm_info = localtime(&timer);

  strftime(date, 25, "%Y:%m:%d %H:%M:%S", tm_info);
  fprintf(fp, date);
  fprintf(fp, "|");
  fprintf(fp, message);
  fprintf(fp, "\n");

  fclose(fp);
  usleep(1000);
}

void write_data_log(Site* site) {

  FILE* fp = site->logger->dataLOG->fp;
  char *filename = site->logger->dataLOG->filename;
  fp = fopen(filename, "a");
  if (!fp) {
    fprintf(stderr, "could not open log file %s", filename);
  }

  time_t timer;
  struct tm* tm_info;
  char date[50], str[80] = "";
  int event_t = 1;           // event type, тип события

  timer = time(NULL);
  tm_info = localtime(&timer);

  strftime(date, 25, "%Y:%m:%d %H:%M:%S", tm_info);
  fprintf(fp, date);
  fprintf(fp, "|");
  sprintf(str, "%d", event_t);
  fprintf(fp, str);
  fprintf(fp, "|");

  memset(str, 0, sizeof(str));
  sprintf(str, "%2.2f", site->temp_in);
  fprintf(fp, str);
  fprintf(fp, ";");

  memset(str, 0, sizeof(str));
  sprintf(str, "%2.2f", site->temp_out);
  fprintf(fp, str);
  fprintf(fp, ";");

  memset(str, 0, sizeof(str));
  sprintf(str, "%2.2f", site->temp_mix);
  fprintf(fp, str);
  fprintf(fp, ";");

  memset(str, 0, sizeof(str));
  sprintf(str, "%2.2f", site->temp_evapor1);
  fprintf(fp, str);
  fprintf(fp, ";");

  memset(str, 0, sizeof(str));
  sprintf(str, "%2.2f", site->temp_evapor2);
  fprintf(fp, str);
  fprintf(fp, ";");

  memset(str, 0, sizeof(str));
  sprintf(str, "%d", site->tacho1_t);
  fprintf(fp, str);
  fprintf(fp, ";");

  memset(str, 0, sizeof(str));
  sprintf(str, "%d", site->tacho2_t);
  fprintf(fp, str);
  fprintf(fp, ";");

  fprintf(fp, "\n");
  fclose(fp);
}

int close_filelog(FileLogWriter* flog) {
  fclose(flog->fp);
  return 0;
}
