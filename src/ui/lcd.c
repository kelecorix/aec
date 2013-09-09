#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "lcd.h"

const unsigned char rusTable[64] = { 0x41, 0xA0, 0x42, 0xA1, 0xE0, 0x45, 0xA3,
    0xA4, 0xA5, 0xA6, 0x4B, 0xA7, 0x4D, 0x48, 0x4F, 0xA8, 0x50, 0x43, 0x54,
    0xA9, 0xAA, 0x58, 0xE1, 0xAB, 0xAC, 0xE2, 0xAD, 0xAE, 0x62, 0xAF, 0xB0,
    0xB1, 0x61, 0xB2, 0xB3, 0xB4, 0xE3, 0x65, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA,
    0xBB, 0xBC, 0xBD, 0x6F, 0xBE, 0x70, 0x63, 0xBF, 0x79, 0xE4, 0x78, 0xE5,
    0xC0, 0xC1, 0xE6, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7 };

static int fd;
static char *fileName = "/dev/i2c-1";
static int lcd_connected = -1; // -1 = unknown, 0 = not connected, 1 = connected

LCD* lcd_new( addr) {
  LCD* lcd = malloc(sizeof(LCD));
  lcd->addres = addr;
  return lcd;
}

void LCD_setup(LCD* lcd) {
  if ((fd = open(fileName, O_RDWR)) < 0) {
    printf("Failed to open the i2c bus\n");
    lcd_connected = 0;
    return;
  }

  int addr = lcd->addres;
  if (ioctl(fd, I2C_SLAVE, addr) < 0) {
    printf("Failed to acquire bus access and/or talk to slave.\n");
    lcd_connected = 0;
    return;
  }
  lcd_connected = 1;
  lcd_reset(lcd);
  lcd_init(lcd);
}

void write_lcd(LCD* lcd, int bits) {
  PutBitsOnPins(lcd, bits + LCD_EN);
  PutBitsOnPins(lcd, bits);
  usleep(500);
}

void lcd_string(char *s) {
  int i;
  for (i = 0; i < strlen(s); i++) {
    write_char(s[i]);
  }
}

void lcd_line(LCD* lcd, char *s, int c) {
  //c линия в какую будем писать
  int i;
  int wyw_s = 0;
  unsigned char zap;
  cursor_go(c);
  //printf("%s\n",s);
  for (i = 0; i < strlen(s); i++) {
    if (((int) s[i] < 32) || ((int) s[i] > 125)) {
      //printf("%x %x\n",(unsigned char)s[i],(unsigned char)s[i+1]);
      if ((unsigned char) s[i] == 0xd0) {
        //printf("%x %x\n",(unsigned char)s[i],(unsigned char)s[i+1]);
        zap = rusTable[(unsigned char) s[i + 1] - 0x90];
        if ((unsigned char) s[i + 1] == 0x81) {
          zap = 0xa2;
        }

        //printf("index_mas = %d",(unsigned char)s[i+1] - 0x90);
        write_char(zap);
        wyw_s++;
        i++;
      }
      if ((unsigned char) s[i] == 0xd1) {
        //printf("%x %x\n",(unsigned char)s[i],(unsigned char)s[i+1]);
        zap = rusTable[(unsigned char) s[i + 1] - 0x50];
        if ((unsigned char) s[i + 1] == 0x91) {
          zap = 0xb5;
        }

        write_char(zap);
        wyw_s++;
        i++;
      }
      if ((unsigned char) s[i] == 0xc2) {
        //printf("%x %x\n",(unsigned char)s[i],(unsigned char)s[i+1]);
        //zap = rusTable[(unsigned char)s[i+1] - 0x50];
        if ((unsigned char) s[i + 1] == 0xb0) {
          zap = 0xdf;
        }
        write_char(zap);
        wyw_s++;
        i++;
      }

    } else {
      write_char(s[i]);
      wyw_s++;
    }
  }
  //printf("wyw_s = %d\n",wyw_s);
  if (wyw_s < 17) {
    for (i = 0; i < 16 - wyw_s; i++) {
      write_char(' ');
    }
  }
}

void write_char(LCD* lcd, char letter) {

  write_lcd(lcd, (((int) letter >> 4) & 0x0F) | LCD_RS);
  write_lcd(lcd, ((int) letter & 0x0F) | LCD_RS);
}

void PutBitsOnPins(LCD* lcd, char bits) {
  if (lcd_connected == -1) {
    LCD_setup(lcd);
  }
  if (lcd_connected == 1) {
    char buf[1];
    //printf("%0x\n",bits);
    buf[0] = bits;
    if (write(fd, buf, 1) != 1) {
      printf("Failed to write to the i2c bus. **lcd.c**\n");
      lcd_connected = 0;
    }
  }
}

void lcd_clear(LCD* lcd) {
  write_nibbles(CMD_CAH);
}

void lcd_reset(LCD* lcd) {
  PutBitsOnPins(lcd, 0xFF);
  usleep(5000);
  PutBitsOnPins(lcd, 0x03 + LCD_EN);
  PutBitsOnPins(lcd, 0x03);
  usleep(5000);
  PutBitsOnPins(lcd, 0x03 + LCD_EN);
  PutBitsOnPins(lcd, 0x03);
  usleep(500);
  PutBitsOnPins(lcd, 0x03 + LCD_EN);
  PutBitsOnPins(lcd, 0x03);
  usleep(500);
  PutBitsOnPins(lcd, 0x02 + LCD_EN);
  PutBitsOnPins(lcd, 0x02);
  usleep(500);
}

void lcd_init(LCD* lcd) {
  write_nibbles(CMD_SIL | SIL_N);
  write_nibbles(CMD_EDC);
  write_nibbles(CMD_CAH);
  write_nibbles(CMD_SCMD | SCMD_ID);
  write_nibbles(CMD_EDC | EDC_D);
}

void write_nibbles(LCD* lcd, int bits) {
  write_lcd(lcd, (bits >> 4) & 0x0F);
  write_lcd(lcd, bits & 0x0F);
  usleep(500);
}

void cursor_go(LCD* lcd, int str) {

  int DDRAMAddr;
  switch (str) {
    case 0:
      DDRAMAddr = LCD_LINE0_DDRAMADDR;
      break;
    case 1:
      DDRAMAddr = LCD_LINE1_DDRAMADDR;
      break;
    case 2:
      DDRAMAddr = LCD_LINE2_DDRAMADDR;
      break;
    case 3:
      DDRAMAddr = LCD_LINE3_DDRAMADDR;
      break;
    default:
      DDRAMAddr = LCD_LINE0_DDRAMADDR;

  }

  // set data address

  //LCDsendCommand(1<<LCD_DDRAM | DDRAMAddr);

  write_nibbles(lcd, 1 << LCD_DDRAM | DDRAMAddr);

}
