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

int g_i2cFile;

// open the Linux device
void i2cOpen() {
  g_i2cFile = open(I2C_FILE_NAME, O_RDWR);
  if (g_i2cFile < 0) {
    printf("Error opening file: %s\n", strerror(errno));
    exit(1);
  }
}

// close the Linux device
void i2cClose() {
  close(g_i2cFile);
}

// set the I2C slave address for all subsequent I2C device transfers
void i2cSetAddress(int address) {
  if (ioctl(g_i2cFile, I2C_SLAVE, address) < 0) {
    printf("Error when setting address: %s\n", strerror(errno));
    exit(1);
  }
}

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

int get_i2c_register(int file,
                     unsigned char addr,
                     unsigned char reg,
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
    messages[0].addr  = addr;
    messages[0].flags = 0;
    messages[0].len   = sizeof(outbuf);
    messages[0].buf   = &outbuf;

    /* The data will get returned in this structure */
    messages[1].addr  = addr;
    messages[1].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
    messages[1].len   = sizeof(inbuf);
    messages[1].buf   = &inbuf;

    /* Send the request to the kernel and get the result back */
    packets.msgs      = messages;
    packets.nmsgs     = 2;
    if(ioctl(file, I2C_RDWR, &packets) < 0) {
        perror("Unable to send data");
        return 1;
    }
    *val = inbuf;

    return 0;
}

void i2cTestHardware() {

  // Fan 0 приточный
  int addrFan1 = 0b00100000, addrFan2 = 0b00100001, addrTh = 0b00100010,
      addrRel = 0x3b;

  i2cOpen();

//  int steps[10] = { 0xFF, 0xED, 0xDF, 0xDE, 0xDC, 0xBF, 0xBE, 0x7F, 0x7E, 0x9F,
//      0x8F };
//
//  i2cSetAddress(addrFan1);
//  set_i2c_register(g_i2cFile, addrFan1, 0, steps[2]);
//  set_i2c_register(g_i2cFile, addrFan2, 0, steps[2]);
//  set_i2c_register(g_i2cFile, addrTh, 0, 0xFF);

  // Тестируем реле или лампочки на RPi
//  int val = 0b00000000;
//  int i, bit =0;
//  for (i = 0; i < 64; i++, bit++) {
//    if ((i % 8) == 0) {
//          val = 0b00000000;
//          bit=0;
//        }
//    val ^= (1 << bit);
//    set_i2c_register(g_i2cFile, addrRel, 0, val);
//
//    sleep(1);
//  }

  // Тестируем реле или лампочки на RPi
   int val = 0xFF;
   char buf[1];
   int i, bit = 2;
   for (i = 0; i < 128; i++) {
     val ^= (1 << bit);
     printf("Value: %x\n", val);
     set_i2c_register(g_i2cFile, addrRel, val, val);
     //sprintf(buf[0],"%s", val);
     //ioctl(g_i2cFile, I2C_SLAVE, addrRel);
     //write(g_i2cFile, buf, 1);
     sleep(2);
   }

//  sleep(10);
//
//  set_i2c_register(g_i2cFile, addrTh, 0, 0xDC);
//  sleep(10);
//  set_i2c_register(g_i2cFile, addrFan1, 0, steps[6]);
//  set_i2c_register(g_i2cFile, addrFan2, 0, steps[6]);
//  set_i2c_register(g_i2cFile, addrTh, 0, 0xED);
//  sleep(10);
//  set_i2c_register(g_i2cFile, addrFan1, 0, steps[0]);
//  set_i2c_register(g_i2cFile, addrFan2, 0, steps[0]);
//  set_i2c_register(g_i2cFile, addrTh, 0, 0x8F);
//
  i2cClose();

}
