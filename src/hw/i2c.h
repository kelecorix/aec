#ifndef I2C_H_
#define I2C_H_

#include <stdio.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#define PCF8574 0b0100000  //PCF8574 ID code
#define PCF8574A 0b0111000 //PCF8574A ID code
#define I2C_FILE_NAME "/dev/i2c-1"

void i2cOpen();
void i2cClose();
void i2cSetAddress(int address);
void i2cTestHardware();

#endif /* I2C_H_ */
