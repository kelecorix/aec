#ifndef NET_LOGGER_H_
#define NET_LOGGER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"

typedef struct NetLogWriter {
  LogWriter base;
  char*   server;
  int     port;
} NetLogWriter;

#endif /* NET_LOGGER_H_ */
