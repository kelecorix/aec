#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "logger.h"
#include "../hw/site.h"
#include "../hw/vent.h"

FileLogWriter* create_filelog(char* filename) {

  FileLogWriter* log = malloc(sizeof(FileLogWriter));
  log->filename = filename;

  return log;
}

void write_log(FILE* fp, char* msg, ...) {

  va_list args;

  time_t timer;
  struct tm* tm_info;
  char date[50];

  timer = time(NULL);
  tm_info = localtime(&timer);

  strftime(date, 25, "%Y:%m:%d %H:%M:%S", tm_info);
  fprintf(fp, date);
  fprintf(fp, "|");
  va_start(args, msg);
  vfprintf(fp, msg, args);
  va_end(args);
  fprintf(fp, "\n");
  fclose(fp);
}

void write_dl(char* msg, int event_t) {

  // event type, тип события
  FILE* fp = site->logger->dataLOG->fp;
  char *filename = site->logger->dataLOG->filename;

  struct tm* tm_info;
  char date[50]="";

  fp = fopen(filename, "a");

  if (!fp)
    fprintf(stderr, "could not open log file %s", filename);

  tm_info = NULL;
  strftime(date, 25, "%Y:%m:%d %H:%M:%S", tm_info);
  fprintf(fp, date);

//  fprintf(fp, "|");
//  sprintf(str, "%d", event_t);
//  fprintf(fp, str);

  fprintf(fp, "|");
  fprintf(fp, msg);

  fprintf(fp, "\n");
  fclose(fp);

}

void write_data_log(Site* site) {

  FILE* fp = site->logger->dataLOG->fp;
  char *filename = site->logger->dataLOG->filename;

  time_t timer;
  struct tm* tm_info;
  char date[50], str[80] = "";
  //int event_t = 1;           // event type, тип события

  fp = fopen(filename, "a");

  if (!fp)
    fprintf(stderr, "could not open log file %s", filename);

  timer = time(NULL);
  tm_info = localtime(&timer);

  strftime(date, 25, "%Y:%m:%d %H:%M:%S", tm_info);
  fprintf(fp, date);
  //пока не выводим тип сообщения
  /*fprintf(fp, "|");
  sprintf(str, "%d", event_t);
  fprintf(fp, str);*/
  //*****************************
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
  //fprintf(fp, ";");

  fprintf(fp, "\n");
  fclose(fp);
}

void logD(FileLogWriter* flog, int pf, char* msg, ...) {
  va_list args;

  flog->fp = fopen(flog->filename, "a");
  time_t timer;
  struct tm* tm_info;
  char date[50];

  timer = time(NULL);
  tm_info = localtime(&timer);

  strftime(date, 25, "%Y:%m:%d %H:%M:%S", tm_info);
  fprintf(flog->fp, date);
  fprintf(flog->fp, "|");
  va_start(args, msg);
  vfprintf(flog->fp, msg, args);
  va_end(args);
  fprintf(flog->fp, "\n");
  fclose(flog->fp);
  // pf - printf flag
  if (pf == 1) {
    va_start(args, msg);
    printf(msg, args);
    va_end(args);
  }
}

// Basic messages
void log_1(char* msg, ...) {
  va_list args;
  FileLogWriter* flog = site->logger->eventLOG;
  if (site->debug >= 1) {
    flog->fp = fopen(flog->filename, "a");
    time_t timer;
    struct tm* tm_info;
    char date[50];

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(date, 25, "%Y:%m:%d %H:%M:%S", tm_info);
    fprintf(site->logger->eventLOG->fp, date);
    fprintf(site->logger->eventLOG->fp, "|");
    va_start(args, msg);
    vfprintf(site->logger->eventLOG->fp, msg, args);
    va_end(args);
    fclose(site->logger->eventLOG->fp);
  }
  // pf - printf flag
  if (site->gpf) {
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
  }
}

// Light Debug
void log_2(char* msg, ...) {
  va_list args;
  FileLogWriter* flog = site->logger->eventLOG;
  if (site->debug >= 2) {
    flog->fp = fopen(flog->filename, "a");
    time_t timer;
    struct tm* tm_info;
    char date[50];

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(date, 25, "%Y:%m:%d %H:%M:%S", tm_info);
    fprintf(site->logger->eventLOG->fp, date);
    fprintf(site->logger->eventLOG->fp, "|");
    va_start(args, msg);
    vfprintf(site->logger->eventLOG->fp, msg, args);
    va_end(args);
    fclose(site->logger->eventLOG->fp);
  }
  // pf - printf flag
  if (site->gpf) {
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
  }
}

// Debug
void log_3(char* msg, ...) {
  va_list args;
  FileLogWriter* flog = site->logger->eventLOG;
  if (site->debug >= 3) {
    site->logger->eventLOG->fp = fopen(flog->filename, "a");
    time_t timer;
    struct tm* tm_info;
    char date[50];

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(date, 25, "%Y:%m:%d %H:%M:%S", tm_info);
    fprintf(site->logger->eventLOG->fp, date);
    fprintf(site->logger->eventLOG->fp, "|");
    va_start(args, msg);
    vfprintf(site->logger->eventLOG->fp, msg, args);
    va_end(args);
    fclose(site->logger->eventLOG->fp);
  }

  // pf - printf flag
  if (site->gpf) {
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
  }
}

// Deep Debug
void log_4(char* msg, ...) {
  va_list args;
  FileLogWriter* flog = site->logger->eventLOG;
  if (site->debug >= 4) {
    site->logger->eventLOG->fp = fopen(flog->filename, "a");
    time_t timer;
    struct tm* tm_info;
    char date[50];

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(date, 25, "%Y:%m:%d %H:%M:%S", tm_info);
    fprintf(site->logger->eventLOG->fp, date);
    fprintf(site->logger->eventLOG->fp, "|");
    va_start(args, msg);
    vfprintf(site->logger->eventLOG->fp, msg, args);
    va_end(args);
    fclose(site->logger->eventLOG->fp);

  }

  // pf - printf flag
  if (site->gpf) {
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
  }
}
