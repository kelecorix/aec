#ifndef NET_LOGGER_H_
#define NET_LOGGER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"

typedef struct NetLogWriter {
  char* server;
  int port;
} NetLogWriter;

NetLogWriter* create_netlog(char* server, int port);

#endif /* NET_LOGGER_H_ */
