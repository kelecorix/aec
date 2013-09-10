#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"

NetLogWriter* create_netlog(char* server, int port){

  NetLogWriter* log = malloc(sizeof(NetLogWriter));
  log->server = server;
  log->port = port;

  return log;

}
