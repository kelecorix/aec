#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include "ui.h"
#include "../hw/i2c.h"

KB* kb_new(int addr) {
  printf("создадим клавиатуру\n");
  KB* kb = malloc(sizeof(KB));
  kb->address = addr;
  kb->fd = 0;
  kb->connect = -1;
  printf("пререзет\n");
  reset_kb(kb);

  return kb;
}

void reset_kb(KB* kb){
  printf("врезете\n");

  if ((kb->fd = open(I2C_FILE_NAME, O_RDWR)) < 0) {
    log_1("Failed to open the i2c bus\n");
    kb->connect = 0;
  } else {
    kb->connect = 1;
  }
  printf("врезете2 %d, %x \n", kb->fd, kb->address);
  if (ioctl(kb->fd, I2C_SLAVE, kb->address) < 0) {
    log_1("Failed to acquire bus access and/or talk to slave.\n");
    kb->connect = 0;
  } else {
    kb->connect = 1;
  }
}
