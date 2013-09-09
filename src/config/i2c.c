#include <stdio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

// I2C Linux device handle
int g_i2cFile;

// open the Linux device
void i2cOpen()
{
  g_i2cFile = open("/dev/i2c-0", O_RDWR);
  if (g_i2cFile < 0) {
    perror("i2cOpen");
    exit(1);
  }
}

// close the Linux device
void i2cClose()
{
  close(g_i2cFile);
}

// set the I2C slave address for all subsequent I2C device transfers
void i2cSetAddress(int address)
{
  if (ioctl(g_i2cFile, I2C_SLAVE, address) < 0) {
    perror("i2cSetAddress");
    exit(1);
  }
}
