#ifndef LCD_H_
#define LCD_H_

#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

// Prototypes declaration
void lcd_reset();
void LCD_setup();
void lcd_init();
void PutBitsOnPins(char bits);
void write_nibbles(int bits);
void write_lcd(int bits);
void write_char(char letter);
void cursor_go(int c);

void lcd_line(char *s, int c);

#endif /* LCD_H_ */
