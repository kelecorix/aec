#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

//Logger
typedef struct Logger {

  int mode; // 0 - typical. 1 - debug, 2 - deep debug

  // Saving data on SD
  int (*dump_full_log)(char *name);
  int (*dump_fail_log)(char *name);
  int (*dump_data_log)(char *name);

  // Sending data over network
  int (*send_full_log)(char *name);
  int (*sent_fail_log)(char *name);
  int (*sent_data_log)(char *name);

} Logger;

#endif /*LOG_H_*/
