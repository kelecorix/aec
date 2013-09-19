#ifndef FILE_LOGGER_H_
#define FILE_LOGGER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"

typedef struct FileLogWriter {
  FILE *fp;
  char* filename;
} FileLogWriter;

FileLogWriter* create_filelog(char* filename);
void write_log(FILE* fp, char* msg, ...);
void write_data_log();
void log0(FileLogWriter* flog, int pf, char* msg, ...);
void log1(char* msg, ...);
void log2(char* msg, ...);
void log3(char* msg, ...);
void log4(char* msg, ...);

#endif /* FILE_LOGGER_H_ */
