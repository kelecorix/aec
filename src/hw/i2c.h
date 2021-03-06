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

#define I2C_FILE_NAME "/dev/i2c-1"

void i2cOpen();
void i2cClose();
void i2cSetAddress(int address);
void i2cTestHardware();
int set_i2c_register(int file, unsigned char addr, unsigned char reg,
    unsigned char value);
int get_i2c_register(int file, unsigned char addr, unsigned char reg,
    unsigned char *val);
int set_relay(int relay, int cond);

extern int g_i2cFile;

#endif /* I2C_H_ */
