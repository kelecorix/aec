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

//Define pin nr for EN and RS
#define LCD_RS 0x20
#define LCD_EN 0x80

//Define LCD settings
#define CMD_SCMD 0x04   //Set Cursor Move Direction:
#define SCMD_ID  0x02   //- Increment the Cursor After Each Byte Written to Display if Set
#define SCMD_S   0x01   //- Shift Display when Byte Written to Display
#define CMD_EDC  0x08   //Enable Display/Cursor
#define EDC_D    0x04   //- Turn Display On
#define EDC_C    0x02   //- Turn Cursor On
#define EDC_B    0x01   //- Cursor Blink On
#define CMD_MCSD 0x10	//Move Cursor/Shift Display
#define MCSD_SC  0x08   //- Display Shift On(1)/Off(0)
#define MCSD_RL  0x04   //- Direction of Shift Right(1)/Left(0)
#define CMD_SIL  0x20   //Set Interface Length
#define SIL_DL   0x10   //- Set Data Interface Length 8
#define SIL_N    0x08   //- Number of Display Lines 2(=4)
#define SIL_F    0x04   //- Character Font 5x10
#define CMD_MCD  0x80   //Move Cursor to Display Address
#define CMD_CAH	 0x01   //Clear and Home
#define CMD_HME  0x02	  //Move home
// cursor position to DDRAM mapping

#define LCD_LINE0_DDRAMADDR		0x00
#define LCD_LINE1_DDRAMADDR		0x40
#define LCD_LINE2_DDRAMADDR		0x10
#define LCD_LINE3_DDRAMADDR		0x50
#define LCD_DDRAM           7	//DB7: set DD RAM address
static int fd;
static char *fileName = "/dev/i2c-1";
static int address = 0x3C;
static int lcd_connected = -1; // -1 = unknown, 0 = not connected, 1 = connected

void LCD_setup() {
  if ((fd = open(fileName, O_RDWR)) < 0) {
    printf("Failed to open the i2c bus\n");
    lcd_connected = 0;
    return;
  }

  if (ioctl(fd, I2C_SLAVE, address) < 0) {
    printf("Failed to acquire bus access and/or talk to slave.\n");
    lcd_connected = 0;
    return;
  }
  lcd_connected = 1;
  lcd_reset();
  lcd_init();
}

void write_lcd(int bits) {
  PutBitsOnPins(bits + LCD_EN);
  PutBitsOnPins(bits);
  usleep(500);
}

void lcd_string(char *s) {
  int i;
  for (i = 0; i < strlen(s); i++) {
    write_char(s[i]);
  }
}

void lcd_line(char *s, int c) {
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

void write_char(char letter) {

  write_lcd((((int) letter >> 4) & 0x0F) | LCD_RS);
  write_lcd(((int) letter & 0x0F) | LCD_RS);
}

void PutBitsOnPins(char bits) {
  if (lcd_connected == -1) {
    LCD_setup();
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

void lcd_clear() {
  write_nibbles(CMD_CAH);
}

void lcd_reset() {
  PutBitsOnPins(0xFF);
  usleep(5000);
  PutBitsOnPins(0x03 + LCD_EN);
  PutBitsOnPins(0x03);
  usleep(5000);
  PutBitsOnPins(0x03 + LCD_EN);
  PutBitsOnPins(0x03);
  usleep(500);
  PutBitsOnPins(0x03 + LCD_EN);
  PutBitsOnPins(0x03);
  usleep(500);
  PutBitsOnPins(0x02 + LCD_EN);
  PutBitsOnPins(0x02);
  usleep(500);
}

void lcd_init() {
  write_nibbles(CMD_SIL | SIL_N);
  write_nibbles(CMD_EDC);
  write_nibbles(CMD_CAH);
  write_nibbles(CMD_SCMD | SCMD_ID);
  write_nibbles(CMD_EDC | EDC_D);
}

void write_nibbles(int bits) {
  write_lcd((bits >> 4) & 0x0F);
  write_lcd(bits & 0x0F);
  usleep(500);
}

void cursor_go(int str) {

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

  write_nibbles(1 << LCD_DDRAM | DDRAMAddr);

}
