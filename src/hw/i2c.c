#include <stdio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include "i2c.h"
#include "site.h"
#include "throttle.h"
#include "vent.h"

int g_i2cFile;

// open the Linux device
/*
 *
 *
 *
 */
void i2cOpen() {
  g_i2cFile = open(I2C_FILE_NAME, O_RDWR);
  if (g_i2cFile < 0) {
    printf("Error opening file: %s\n", strerror(errno));
    exit(1);
  }
}

// close the Linux device
/*
 *
 *
 *
 */
void i2cClose() {
  close(g_i2cFile);
}

// set the I2C slave address for all subsequent I2C device transfers
/*
 *
 *
 *
 */
void i2cSetAddress(int address) {
  if (ioctl(g_i2cFile, I2C_SLAVE, address) < 0) {
    printf("Error when setting address: %s\n", strerror(errno));
    exit(1);
  }
}

/*
 *
 *
 *
 */
int set_i2c_register(int file, unsigned char addr, unsigned char reg,
    unsigned char value) {

  unsigned char outbuf[2];
  struct i2c_rdwr_ioctl_data packets;
  struct i2c_msg messages[1];

  messages[0].addr = addr;
  messages[0].flags = 0;
  messages[0].len = sizeof(outbuf);
  messages[0].buf = outbuf;

  /* The first byte indicates which register we'll write */
  outbuf[0] = reg;

  outbuf[1] = value;

  packets.msgs = messages;
  packets.nmsgs = 1;
  if (ioctl(file, I2C_RDWR, &packets) < 0) {
    perror("Unable to send data");
    return 1;
  }

  return 0;
}

/*
 *
 *
 *
 */
int get_i2c_register(int file, unsigned char addr, unsigned char reg,
    unsigned char *val) {

  unsigned char inbuf, outbuf;
  struct i2c_rdwr_ioctl_data packets;
  struct i2c_msg messages[2];

  /*
   * In order to read a register, we first do a "dummy write" by writing
   * 0 bytes to the register we want to read from.  This is similar to
   * the packet in set_i2c_register, except it's 1 byte rather than 2.
   */
  outbuf = reg;
  messages[0].addr = addr;
  messages[0].flags = 0;
  messages[0].len = sizeof(outbuf);
  messages[0].buf = &outbuf;

  /* The data will get returned in this structure */
  messages[1].addr = addr;
  messages[1].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
  messages[1].len = sizeof(inbuf);
  messages[1].buf = &inbuf;

  /* Send the request to the kernel and get the result back */
  packets.msgs = messages;
  packets.nmsgs = 2;
  if (ioctl(file, I2C_RDWR, &packets) < 0) {
    perror("Unable to send data");
    return 1;
  }
  *val = inbuf;

  return 0;
}

/*
 *
 *
 *
 */
int get_i2c_register_adc(int file, unsigned char addr, unsigned char reg,
    unsigned char *val) {

  unsigned char inbuf, outbuf;
  struct i2c_rdwr_ioctl_data packets;
  struct i2c_msg messages[2];

  /*
   * In order to read a register, we first do a "dummy write" by writing
   * 0 bytes to the register we want to read from.  This is similar to
   * the packet in set_i2c_register, except it's 1 byte rather than 2.
   */
  outbuf = reg;
  messages[0].addr = addr;
  messages[0].flags = 0;
  messages[0].len = sizeof(outbuf);
  messages[0].buf = &outbuf;

  /* The data will get returned in this structure */
  messages[1].addr = addr;
  messages[1].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
  messages[1].len = sizeof(2*inbuf);
  messages[1].buf = &inbuf;

  /* Send the request to the kernel and get the result back */
  packets.msgs = messages;
  packets.nmsgs = 2;
  if (ioctl(file, I2C_RDWR, &packets) < 0) {
    perror("Unable to send data");
    return 1;
  }
  *val = inbuf;

  return 0;
}

void test_relay(){

  char *a_relay = getStr(site->cfg, (void *) "a_relay");

  i2cOpen();

  set_i2c_register(g_i2cFile, a_relay, 1, 1);


  i2cClose();


}

/*
 *
 *
 *
 */
void i2cTestHardware() {

  test_relay();
  //test_vents();
  //test_throttle();
}


