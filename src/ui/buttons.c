#include <stdio.h>
#include <unistd.h>
#include <linux/input.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>

int btn;
int fd;
int povtor = 2;
int povtor_old = 2;

char *BUTTONS = "/dev/i2c-1";
int BTNaddress = 0x3D; // TODO: Читать из конфига!!
int buttonsConnected = -1; // -1 = unknown, 0 = not connected, 1 = connected
int preButton = 0;

void initButtons() {
  if ((btn = open(BUTTONS, O_RDWR)) < 0) {
    printf("Failed to open the i2c bus\n");
    buttonsConnected = 0;
    return;
  }
  if (ioctl(btn, I2C_SLAVE, BTNaddress) < 0) {
    printf("Failed to acquire bus access and/or talk to slave.\n");
    buttonsConnected = 0;
    return;
  }
  buttonsConnected = 1;
}

int checkButton() {
  char buf[1];
  int button = 0;

  if (buttonsConnected == -1) {
    initButtons();
  }
  if (buttonsConnected == 0) {
    return 999;
  }

  if (buttonsConnected == 1) {
    if (read(btn, buf, 1) != 1) {
      printf("BUTTONS Error reading from i2c\n");
      buttonsConnected = 0;
    } else {
      //printf("%0x\n",(uint)buf[0]);
      switch ((uint) buf[0]) {
      case 127:
        if (preButton == 1) {
          if (povtor != 0) {
            povtor--;
          } else {
            povtor = povtor_old;
            preButton = 0;
          }
        }
        if (preButton != 1) {
          preButton = 1;
          button = 1;
        }
        break;

      case 191:
        if (preButton != 2) {
          preButton = 2;
          button = 2;
        }
        break;

      case 223:
        if (preButton == 3) {
          if (povtor != 0) {
            povtor--;
          } else {
            povtor = povtor_old;
            preButton = 0;
          }
        }
        if (preButton != 3) {
          preButton = 3;
          button = 3;
        }
        break;

      case 239:
        if (preButton != 4) {
          preButton = 4;
          button = 4;
        }
        break;

      case 247:
        if (preButton != 5) {
          preButton = 5;
          button = 5;
        }
        break;

      default:
        preButton = 0;

      }
    }
  }
  return button;
}
