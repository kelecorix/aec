#ifndef MENU_H_
#define MENU_H_

#include <stdio.h>
#include <time.h>

#include "ui.h"
#include "lcd.h"
#include "keyboard.h"
#include "menu.h"
#include "../hw/site.h"
#include "../config/config.h"

typedef struct Node{
  int         id;
  //Node       Parent;
  char*       Text;
  int         Min;
  int         Max;
  int         val;
} Node;


typedef struct Tree{
  Node* root; // головной элемент
  Node* curr; // текущий элемент
} Menu;


#endif /*MENU_H_*/




