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

void write_log(FileLogWriter* flog, char* message, ...) {

  va_list args;
  flog->fp = fopen(flw->filename, "a");

  if (!flog->fp)
    fprintf(stderr, "could not open log file %s", flog->filename);

  time_t timer;
  struct tm* tm_info;
  char date[50];
  int event_t; // event type, тип события

  timer = time(NULL);
  tm_info = localtime(&timer);

  strftime(date, 25, "%Y:%m:%d %H:%M:%S", tm_info);
  fprintf(flog->fp, date);
  fprintf(flog->fp, "|");
  fprintf(flog->fp, message, args);
  fprintf(flog->fp, "\n");

  fclose(flog->fp);
}

void write_data_log(Site* site) {

  FILE* fp = site->logger->dataLOG->fp;
  char *filename = site->logger->dataLOG->filename;
 
  time_t timer;
  struct tm* tm_info;
  char date[50], str[80] = "";
  int event_t = 1;           // event type, тип события

  fp = fopen(filename, "a");
  
  if (!fp)
    fprintf(stderr, "could not open log file %s", filename);

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

void log0(FileLogWriter* flog, int pf, char* msg, ...){
  va_list args;
  if (site->debug >= 1) 
    write_log(flog, msg, args);
  
  // pf - printf flag
  if (site->gpf)
    printf(msg, args);

}


// Basic messages
void log1(char* msg, ...){
  va_list args;
  if (site->debug >= 1) 
    write_log(site->logger->eventLOG, msg, args);
  
  // pf - printf flag
  if (site->gpf)
    printf(msg, args);

}

// Light Debug
void log2(char* msg, ...){
  va_list args;
  if (site->debug >= 2) 
    write_log(site->logger->eventLOG, msg, args);

  // pf - printf flag
  if (site->gpf)
    printf(msg, args);
}

// Debug
void log3(FileLogWriter* flog, int pf, char* msg, ...){
  va_list args;
  if (site->debug >= 3) 
    write_log(site->logger->eventLOG, msg, args);
   
  // pf - printf flag
  if (site->gpf)
    printf(msg, args);
}

// Deep Debug
void log4(FileLogWriter* flog, int pf, char* msg, ...){
  va_list args;
  if (site->debug >= 4) 
    write_log(site->logger->eventLOG, msg, args);

  // pf - printf flag
  if (site->gpf)
    printf(msg, args);
}
