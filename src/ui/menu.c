//#include <stdio.h>
//#include <stdint.h>
//#include <time.h>
//
//#include "ui.h"
//#include "lcd.h"
//#include "keyboard.h"
//#include "menu.h"
//#include "msg.h"
//#include "../hw/site.h"
//#include "../config/config.h"
//
//menuItem* selectedMenuItem; // текущий пункт меню
//int mode; // режим редактирования или нет
//
//void init_menu(){
//
//
//}
//
//uint8_t start_menu(){
//
//  return 0;
//}
//
//void menuChange(menuItem* NewMenu)
//{
//  if ((void*)NewMenu == (void*)&NE)
//    return;
//
//  selectedMenuItem = NewMenu;
//}
//
//void onKeyClick(menuItem* Menu){
//
//
//}
//
//void menuText() {
//
//
//}
//
//uint8_t keyMenu(msg_par par) {
//  switch (par) {
//  case 0: {
//    return 1;
//  }
//  case KEY_UP: {
//    menuChange(PREVIOUS); // подъем значения
//    break;
//  }
//  case KEY_DOWN: {
//
//    // вход в подменю
//    // вход в режим редактирования
//
//
//    break;
//  }
//  case KEY_RIGHT: {
//
//    menuChange(NEXT); // следующий пункт
//    //сохранить выбор
//    // выйти из редактирования
//    break;
//  }
//  case KEY_LEFT: {   //отмена выбора возврат
//    menuChange(PREVIOUS); // предыдущий пункт
//    break;
//   }
//  }
//  dispMenu(0);
//  return (1);
//}
