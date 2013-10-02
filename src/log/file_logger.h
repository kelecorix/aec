#ifndef FILE_LOGGER_H_
#define FILE_LOGGER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

/*
 *
 *
 *
 */
typedef struct FileLogWriter {
  FILE *fp;
  char* filename;
} FileLogWriter;

FileLogWriter* create_filelog(char* filename);
void write_log(FILE* fp, char* msg, ...);
void write_data_log();
void write_dl(char* msg,  int event_t );
void logD(FileLogWriter* flog, int pf, char* msg, ...);
void log_1(char* msg, ...);
void log_2(char* msg, ...);
void log_3(char* msg, ...);
void log_4(char* msg, ...);


#endif /* FILE_LOGGER_H_ */
