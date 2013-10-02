#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "logger.h"
#include "../hw/site.h"
#include "../hw/vent.h"
#include "file_logger.h"

/*
 *
 *
 *
 */
FileLogWriter* create_filelog(char* filepath) {

  FileLogWriter* log = malloc(sizeof(FileLogWriter));
  log->filepath = filepath;

  return log;
}

/*
 *
 *
 *
 */
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


/*
 *
 *
 *
 */
void write_data_log() {

  FILE* fp = gcfg->logger->dataLOG->fp;
  char *filepath = gcfg->logger->dataLOG->filepath;

  time_t timer;
  struct tm* tm_info;
  char date[50], str[80] = "";
  //int event_t = 1;           // event type, тип события

  fp = fopen(filepath, "at");

  if (!fp)
    fp = fopen(gcfg->logger->eventLOG->filepath, "wt");

  if (!fp) {
    printf("can not open event log for writing.\n");
    return;   // bail out if we can't log
  }

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

/*
 *
 *
 *
 */
void logD(FileLogWriter* flog, int pf, char* msg, ...) {
  va_list args;

  flog->fp = fopen(flog->filepath, "a");
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
/*
 *
 *
 *
 */
void log_1(char* msg, ...) {
  va_list args;
  FileLogWriter* flog = gcfg->logger->eventLOG;
  if (gcfg->debug >= 1) {
    flog->fp = fopen(flog->filepath, "at");

    if (!flog->fp)
      flog->fp = fopen(gcfg->logger->eventLOG->filepath, "wt");

    if (!flog->fp) {
      printf("can not open event log for writing.\n");
      return;   // bail out if we can't log
    }
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
    fclose(flog->fp);
  }
  // pf - printf flag
  if (gcfg->gpf) {
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
  }
}

// Light Debug
/*
 *
 *
 *
 */
void log_2(char* msg, ...) {
  va_list args;
  FileLogWriter* flog = gcfg->logger->eventLOG;
  if (gcfg->debug >= 2) {
    flog->fp = fopen(flog->filepath, "at");

    if (!flog->fp)
      flog->fp = fopen(gcfg->logger->eventLOG->filepath, "wt");

    if (!flog->fp) {
      printf("can not open event log for writing.\n");
      return;   // bail out if we can't log
    }
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
    fclose(flog->fp);

  }
  // pf - printf flag
  if (gcfg->gpf) {
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
  }
}

// Debug
/*
 *
 *
 *
 */
void log_3(char* msg, ...) {
  va_list args;
  FileLogWriter* flog = gcfg->logger->eventLOG;
  if (gcfg->debug >= 3) {
    flog->fp = fopen(flog->filepath, "at");

    if (!flog->fp)
      flog->fp = fopen(flog->filepath, "wt");

    if (!flog->fp) {
      printf("can not open event log for writing.\n");
      return;   // bail out if we can't log
    }
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
    fclose(flog->fp);
  }

  // pf - printf flag
  if (gcfg->gpf) {
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
  }
}

// Deep Debug
/*
 *
 *
 *
 */
void log_4(char* msg, ...) {
  va_list args;
  FileLogWriter* flog = gcfg->logger->eventLOG;
  if (gcfg->debug >= 4) {
    flog->fp = fopen(flog->filepath, "at");

    if (!flog->fp)
      flog->fp = fopen(flog->filepath, "wt");

    if (!flog->fp) {
      printf("can not open event log for writing.\n");
      return;   // bail out if we can't log
    }
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
    fclose(flog->fp);

  }

  // pf - printf flag
  if (gcfg->gpf) {
    va_start(args, msg);
    vprintf(msg, args);
    va_end(args);
  }
}
