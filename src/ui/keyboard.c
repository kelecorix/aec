#include "buttons.h"
#include "ui.h"

KB* kb_new( addr) {

  KB* kb = malloc(sizeof(KB));
  kb->address = addr;
  return kb;
}

int keyboard(int menu_current) {

  //printf("current menu:  %d\n", menu_current);
  switch (checkButton()) {
    case 4:
      //меню при повторном нажатии выход из меню
      if (menu_current == 11 || menu_current == 12 || menu_current == 13) {
        menu_current = menu_current * 10;
      }
      if (menu_current == 21 || menu_current == 22 || menu_current == 23) {
        menu_current = menu_current * 10;
      }
      if (menu_current == 31 || menu_current == 32 || menu_current == 33) {
        menu_current = menu_current * 10;
      }
      if (menu_current == 41 || menu_current == 42 || menu_current == 43) {
        menu_current = menu_current * 10;
      }
      if (menu_current == 10 || menu_current == 20 || menu_current == 30
          || menu_current == 40) {
        menu_current++;
      }
      if (menu_current == 0 || menu_current == 2000) {
        menu_current = 10;
      }
      break;
    case 2:
      //ок выбор меню, переход к редактиррованию, подтверждение после редактирования
      //что то не так с описанием
      //display_mode = mode_MEM;
      //display_mode = mode_OSN;
      if (menu_current == 11 || menu_current == 12 || menu_current == 13) {
        menu_current = 10;
        break;

      }
      if (menu_current == 21 || menu_current == 22 || menu_current == 23) {
        menu_current = 20;
        break;
      }
      if (menu_current == 31 || menu_current == 32 || menu_current == 33) {
        menu_current = 30;
        break;
      }
      if (menu_current == 41 || menu_current == 42 || menu_current == 43) {
        menu_current = 40;
        break;
      }
      if (menu_current > 100) {
        menu_current = menu_current / 10;
        break;
      }
      if (menu_current == 10 || menu_current == 20 || menu_current == 30
          || menu_current == 40) {
        menu_current = 0;
      }
      break;
    case 3:
      //вверх
      //display_mode = mode_NET;
      if (menu_current == 20 || menu_current == 30 || menu_current == 40) {
        menu_current = menu_current - 10;
      }
      if (menu_current == 12 || menu_current == 13) {
        menu_current--;
      }
      if (menu_current == 22 || menu_current == 23) {
        menu_current--;
      }
      if (menu_current == 32 || menu_current == 33) {
        menu_current--;
      }
      if (menu_current == 42 || menu_current == 43) {
        menu_current--;
      }
      if (menu_current == 110) {
        //??
      }
      break;
    case 1:
      //вниз
      //display_mode = mode_UPT;
      if (menu_current == 10 || menu_current == 20 || menu_current == 30) {
        menu_current = menu_current + 10;
      }
      if (menu_current == 11 || menu_current == 12) {
        menu_current++;
      }
      if (menu_current == 21 || menu_current == 22) {
        menu_current++;
      }
      if (menu_current == 31 || menu_current == 32) {
        menu_current++;
      }
      if (menu_current == 41 || menu_current == 42) {
        menu_current++;
      }
      if (menu_current == 110) {
        //??
      }
      break;
  }

  return menu_current;

}
