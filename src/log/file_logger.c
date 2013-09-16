#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"
#include "../hw/site.h"
#include "../nw/vent.h"

FileLogWriter* create_filelog(char* filename) {

  FileLogWriter* log = malloc(sizeof(FileLogWriter));
  log->filename = filename;

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

  char *a_tacho_in = getStr(site->cfg, (void *) "a_tacho_flow_in");
  char *a_tacho_out = getStr(site->cfg, (void *) "a_tacho_flow_out");

  site->tacho1_t = i2c_get_tacho_step(site->vents[0],
      strtol(a_tacho_in, NULL, 16));
  site->tacho2_t = i2c_get_tacho_step(site->vents[1],
      strtol(a_tacho_out, NULL, 16));

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
  sprintf(str, "%2.2f", site->tacho1_t);
  fprintf(fp, str);
  fprintf(fp, ";");

  memset(str, 0, sizeof(str));
  sprintf(str, "%2.2f", site->tacho2_t);
  fprintf(fp, str);
  fprintf(fp, ";");

  fprintf(fp, "\n");
  fclose(fp);
}

int close_filelog(FileLogWriter* flog) {
  fclose(flog->fp);
  return 0;
}
