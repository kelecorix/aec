#ifndef LCD_H_
#define LCD_H_

#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "../hw/site.h"

// Define pin nr for EN and RS
#define LCD_RS 0x20
#define LCD_EN 0x80

// Define LCD settings
#define CMD_SCMD 0x04   //Set Cursor Move Direction:
#define SCMD_ID  0x02   //- Increment the Cursor After Each Byte Written to Display if Set
#define SCMD_S   0x01   //- Shift Display when Byte Written to Display
#define CMD_EDC  0x08   //Enable Display/Cursor
#define EDC_D    0x04   //- Turn Display On
#define EDC_C    0x02   //- Turn Cursor On
#define EDC_B    0x01   //- Cursor Blink On
#define CMD_MCSD 0x10 //Move Cursor/Shift Display
#define MCSD_SC  0x08   //- Display Shift On(1)/Off(0)
#define MCSD_RL  0x04   //- Direction of Shift Right(1)/Left(0)
#define CMD_SIL  0x20   //Set Interface Length
#define SIL_DL   0x10   //- Set Data Interface Length 8
#define SIL_N    0x08   //- Number of Display Lines 2(=4)
#define SIL_F    0x04   //- Character Font 5x10
#define CMD_MCD  0x80   //Move Cursor to Display Address
#define CMD_CAH  0x01   //Clear and Home
#define CMD_HME  0x02   //Move home
// Cursor position to DDRAM mapping
#define LCD_LINE0_DDRAMADDR   0x00
#define LCD_LINE1_DDRAMADDR   0x40
#define LCD_LINE2_DDRAMADDR   0x10
#define LCD_LINE3_DDRAMADDR   0x50
#define LCD_DDRAM             7 //DB7: set DD RAM address
typedef struct LCD {

  Site* site;
  int addres;

  void (*lcd_init)(struct LCD*);
  void (*LCD_setup)(struct LCD*);
  void (*lcd_line)(struct LCD*, char *s, int c);
  void (*PutBitsOnPins)(struct LCD*, char bits);
  void (*cursor_go)(struct LCD*, int c);
  void (*write_nibbles)(struct LCD*, int bits);
  void (*write_lcd)(struct LCD*, int bits);
  void (*write_char)(struct LCD*, char letter);
  void (*lcd_clear)(struct LCD*);
  void (*lcd_reset)(struct LCD*);

} LCD;

#endif /* LCD_H_ */
