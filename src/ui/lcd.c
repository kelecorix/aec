#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "../hw/site.h"
#include "../hw/i2c.h"
#include "lcd.h"


const unsigned char rusTable[64] = { 0x41, 0xA0, 0x42, 0xA1, 0xE0, 0x45, 0xA3, 0xA4, 0xA5, 0xA6, 0x4B, 0xA7, 0x4D, 0x48, 0x4F, 0xA8, 0x50,
    0x43, 0x54, 0xA9, 0xAA, 0x58, 0xE1, 0xAB, 0xAC, 0xE2, 0xAD, 0xAE, 0x62, 0xAF, 0xB0, 0xB1, 0x61, 0xB2, 0xB3, 0xB4, 0xE3, 0x65, 0xB6,
    0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0x6F, 0xBE, 0x70, 0x63, 0xBF, 0x79, 0xE4, 0x78, 0xE5, 0xC0, 0xC1, 0xE6, 0xC2, 0xC3, 0xC4,
    0xC5, 0xC6, 0xC7 };

/* Функция создания структуры экрана
 * Параметры:
 *            int addr - адрес на шине i2c
 */
Disp* lcd_new(int addr) {
  log_2("выделение памяти\n");
  Disp* lcd = malloc(sizeof(Disp));
  printf("память выделена\n");
  lcd->addr = addr;
  printf("возврат\n");
  return lcd;
}

/* Низкоуровневая функция записи на экран
 */
void send(Disp* lcd, char bits) {
  if (lcd->connect == -1)
    reset(lcd);

  if (lcd->connect == 1) {
    char buf[1];
    buf[0] = bits;
    if (write(lcd->fd, buf, 1) != 1) {
      printf("Failed to write to the i2c bus. **lcd.c**\n");
      lcd->connect = 0;
    }
  }
}

/* Функция записи на экрн
 */
void write_lcd(Disp* lcd, int bits) {
  send(lcd, bits + LCD_EN);
  send(lcd, bits);
  usleep(500);
}

/* Функция записи полубайта на шину i2c
 */
void write_quartets(Disp* lcd, int bits) {
  write_lcd(lcd, (bits >> 4) & 0x0F);
  write_lcd(lcd, bits & 0x0F);
  usleep(500);
}

/* Функция записи символа
 */
void write_char(Disp* lcd, char letter) {
  write_lcd(lcd, (((int) letter >> 4) & 0x0F) | LCD_RS);
  write_lcd(lcd, ((int) letter & 0x0F) | LCD_RS);
}

/* Функция перевода курсора при записи
 */
void set_cursor(Disp* lcd, int str) {

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
  write_quartets(lcd, 1 << LCD_DDRAM | DDRAMAddr);
}

/* Функция последовательной записи символов
 */
void lcd_line(Disp* lcd, char *s, int c) {

  int i;
  int wyw_s = 0;
  unsigned char zap;
  set_cursor(lcd, c);

  for (i = 0; i < strlen(s); i++) {
    if (((int) s[i] < 32) || ((int) s[i] > 125)) {
      if ((unsigned char) s[i] == 0xd0) {
        zap = rusTable[(unsigned char) s[i + 1] - 0x90];
        if ((unsigned char) s[i + 1] == 0x81) {
          zap = 0xa2;
        }
        write_char(lcd, zap);
        wyw_s++;
        i++;
      }
      if ((unsigned char) s[i] == 0xd1) {
        zap = rusTable[(unsigned char) s[i + 1] - 0x50];
        if ((unsigned char) s[i + 1] == 0x91)
          zap = 0xb5;

        write_char(lcd, zap);
        wyw_s++;
        i++;
      }
      if ((unsigned char) s[i] == 0xc2) {
        if ((unsigned char) s[i + 1] == 0xb0)
          zap = 0xdf;

        write_char(lcd, zap);
        wyw_s++;
        i++;
      }
    } else {
      write_char(lcd, s[i]);
      wyw_s++;
    }
  }
  if (wyw_s < 17) {  // ??? Что за магические цифры
    for (i = 0; i < 16 - wyw_s; i++) {
      write_char(lcd, ' ');
    }
  }
}

/* Функция инициализации экрана
 */
void init(Disp* lcd) {

  printf("инициализация экрана\n");
  int fd;
  if ((fd = open(I2C_FILE_NAME, O_RDWR)) < 0) {
    log_1("Failed to open the i2c bus\n");
    lcd->connect = 0;
  } else {
    lcd->connect = 1;
  }

  log_3("LCD addr: %x, connect %d\n", lcd->addr, lcd->connect);
  if (ioctl(fd, I2C_SLAVE, lcd->addr) < 0) {
    log_1("Failed to acquire bus access and/or talk to slave.\n");
    lcd->connect = 0;
  } else {
    lcd->connect = 1;
  }

  write_quartets(lcd, CMD_SIL | SIL_N);
  write_quartets(lcd, CMD_EDC);
  write_quartets(lcd, CMD_CAH);
  write_quartets(lcd, CMD_SCMD | SCMD_ID);
  write_quartets(lcd, CMD_EDC | EDC_D);
}

/* Функция перезагрузки экрана
 */
void reset(Disp* lcd) {
  send(lcd, 0xFF);
  usleep(5000);
  send(lcd, 0x03 + LCD_EN);
  send(lcd, 0x03);
  usleep(5000);
  send(lcd, 0x03 + LCD_EN);
  send(lcd, 0x03);
  usleep(500);
  send(lcd, 0x03 + LCD_EN);
  send(lcd, 0x03);
  usleep(500);
  send(lcd, 0x02 + LCD_EN);
  send(lcd, 0x02);
  usleep(500);
}

/* Функция очистки экрана
 */
void clear(Disp* lcd) {
  write_quartets(lcd, CMD_CAH);
}
