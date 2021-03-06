#ifndef UI_H_
#define UI_H_

#include <stdio.h>
#include <time.h>

#include "ui.h"
#include "lcd.h"
#include "keyboard.h"
#include "../hw/site.h"

#define mode_OSN 1
#define mode_MEM 2
#define mode_NET 3
#define mode_UPT 4
#define mode_intro 5
#define mode_MENU 6

void run_ui();

#endif /*UI_H_*/
