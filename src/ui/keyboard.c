#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include "ui.h"
#include "../hw/i2c.h"

KB* kb_new( addr) {

  KB* kb = malloc(sizeof(KB));
  kb->address = addr;
  kb->connect = -1;

  if ((kb->fd = open(I2C_FILE_NAME, O_RDWR)) < 0) {
    log1("Failed to open the i2c bus\n");
    kb->connect = 0;
  } else {
    kb->connect = 1;
  }
  if (ioctl(kb->fd, I2C_SLAVE, addr) < 0) {
    log1("Failed to acquire bus access and/or talk to slave.\n");
    kb->connect = 0;
  } else {
    kb->connect = 1;
  }

  return kb;
}
