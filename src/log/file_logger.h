#ifndef FILE_LOGGER_H_
#define FILE_LOGGER_H_

/*
 *
 *
 *
 */
typedef struct FileLogWriter {
  FILE *fp;
  char* filepath;
} FileLogWriter;

FileLogWriter* create_filelog(char* filepath);
void write_log(FILE* fp, char* msg, ...);
void write_data_log();
void write_dl(char* msg,  int event_t );
void logD(FileLogWriter* flog, int pf, char* msg, ...);
void log_1(char* msg, ...);
void log_2(char* msg, ...);
void log_3(char* msg, ...);
void log_4(char* msg, ...);


#endif /* FILE_LOGGER_H_ */
