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

typedef struct PROGMEM{
  void       *Next;
  void       *Previous;
  void       *Parent;
  void       *Child;
  uint8_t     Select;
  int         Min;
  int         Max;
  const char  Text[];
} menuItem;

#define MAKE_MENU(Name, Next, Previous, Parent, Child, Select, Min, Max, Text) \
  extern menuItem Next;						     \
  extern menuItem Previous;					     \
  extern menuItem Parent;					     \
  extern menuItem Child;						\
  menuItem Name = {(void*)&Next, (void*)&Previous, (void*)&Parent, (void*)&Child, (uint8_t)Select, {Min}, {Max}, { Text }}


menuItem	Null_Menu = {(void*)0, (void*)0, (void*)0, (void*)0, 0, 0, 0, {0x00}};
 
#define PREVIOUS   ((menuItem*)pgm_read_word(&selectedMenuItem->Previous))
#define NEXT       ((menuItem*)pgm_read_word(&selectedMenuItem->Next))
#define PARENT     ((menuItem*)pgm_read_word(&selectedMenuItem->Parent))
#define CHILD      ((menuItem*)pgm_read_word(&selectedMenuItem->Child))
#define SELECT	   ((pgm_read_byte(&selectedMenuItem->Select))

enum {
    MENU_CANCEL=1,
    MENU_RESET,
    MENU_MODE1,
    MENU_MODE2,
    MENU_MODE3,
    MENU_SENS1,
    MENU_SENS2,
};

//                 NEXT,  PREV,  PAR, CHILD  
MAKE_MENU(m_s1i1, m_s1i2, NE    , NE, m_s2i1, 0, 0, 0, "Температуры");
MAKE_MENU(m_s1i2, m_s1i3, m_s1i1, NE, m_s3i1, 0, 0, 0, "Оборудование");
MAKE_MENU(m_s1i3, m_s1i4, m_s1i2, NE, m_s4i1, 0, 0, 0, "Лог Ошибок");
MAKE_MENU(m_s1i4, m_s1i5, m_s1i3, NE, m_s5i1, 0, 0, 0, "Установка датчиков");
MAKE_MENU(m_s1i5, m_s1i6, m_s1i4, NE, m_s6i1, 0, 0, 0, "Установка времени");
MAKE_MENU(m_s1i6, NE    , m_s1i5, NE, m_s7i1, 0, 0, 0, "Установка интервалов");

// Меню температуры
MAKE_MENU(m_s2i1, m_s2i2, NE    , m_s1i1, NE, 0, 10, 40, "Поддержания");
MAKE_MENU(m_s2i2, m_s2i3, m_s2i1, m_s1i1, NE, 0,  1, 10, "Дельта поддержания");
MAKE_MENU(m_s2i3, m_s2i4, m_s2i2, m_s1i1, NE, 0,-10, 20, "Догрева");
MAKE_MENU(m_s2i4, m_s2i5, m_s2i3, m_s1i1, NE, 0,-10, 20, "Росы улица");
MAKE_MENU(m_s2i5, m_s2i6, m_s2i4, m_s1i1, NE, 0,-10, 20, "Миксер");
MAKE_MENU(m_s2i6, m_s2i7, m_s2i5, m_s1i1, NE, 0,  0, 80, "Аварии");
MAKE_MENU(m_s2i7, m_s2i8, m_s2i6, m_s1i1, NE, 0,  0, 20, "Дельта окружающей");
MAKE_MENU(m_s2i8, m_s2i9, m_s2i7, m_s1i1, NE, 0,  0, 30, "Дельта конд 1");
MAKE_MENU(m_s2i9, NE    , m_s2i8, m_s1i1, NE, 0,  0, 30, "Дельта конд 2");

// Меню оборудование
MAKE_MENU(m_s3i1, m_s3i2, NE    , m_s1i2, NE, 0, 0, 1, "Миксер");  // Да/НЕТ
MAKE_MENU(m_s3i2, m_s3i3, m_s3i1, m_s1i2, NE, 0, 0, 1, "ТЭН");     // Да/НЕТ
MAKE_MENU(m_s3i3, NE    , m_s3i2, m_s1i2, NE, 0, 0, 2, "Кол. кондиционеров");

// Меню Лог Ошибок
MAKE_MENU(m_s4i1, m_s4i2, NE    , m_s1i3, NE, 0, 0, 0, "Просмотреть");
MAKE_MENU(m_s4i2, NE    , m_s4i1, m_s1i3, NE, 0, 0, 0, "Очистить");

// Меню Установка датчиков
MAKE_MENU(m_s5i1, m_s5i2, NE    , m_s1i4, m_s51i1, 0, 0, 0, "Наличие");
MAKE_MENU(m_s5i2, m_s5i3, m_s5i1, m_s1i4, m_s52i1, 0, 0, 0, "Адреса");
MAKE_MENU(m_s5i3, NE    , m_s5i2, m_s1i4, m_s53i1, 0, 0, 0, "Установка");

// Меню Установка датчиков - Наличие
MAKE_MENU(m_s51i1, m_s51i2, NE     , m_s5i1, NE, 0, 0, 0, "Улица");
MAKE_MENU(m_s51i2, m_s51i3, m_s51i1, m_s5i1, NE, 0, 0, 0, "Сайт");
MAKE_MENU(m_s51i3, m_s51i4, m_s51i2, m_s5i1, NE, 0, 0, 0, "Миксер");
MAKE_MENU(m_s51i4, m_s51i5, m_s51i3, m_s5i1, NE, 0, 0, 0, "Кондиционер1");
MAKE_MENU(m_s51i5, NE     , m_s51i4, m_s5i1, NE, 0, 0, 0, "Кондиционер2");

// Меню Установка датчиков - Адреса
MAKE_MENU(m_s52i1, m_s52i2, NE     , m_s5i2, NE, 0, 0, 0, "Улица");
MAKE_MENU(m_s52i2, m_s52i3, m_s52i1, m_s5i2, NE, 0, 0, 0, "Сайт");
MAKE_MENU(m_s52i3, m_s52i4, m_s52i2, m_s5i2, NE, 0, 0, 0, "Миксер");
MAKE_MENU(m_s52i4, m_s52i5, m_s52i3, m_s5i2, NE, 0, 0, 0, "Кондиционер1");
MAKE_MENU(m_s52i5, NE     , m_s52i4, m_s5i2, NE, 0, 0, 0, "Кондиционер2");

// Меню Установка датчиков - Установка
MAKE_MENU(m_s53i1, NE, NE, m_s5i3, m_s5i3, 0, 0, 0, "Установка");

// Меню Установка времени
MAKE_MENU(m_s6i1, m_s6i2, NE    , m_s1i5, NE, 0, 0, 0, "Установка даты");
MAKE_MENU(m_s6i2, NE    , m_s6i1, m_s1i5, NE, 0, 0, 0, "Установка времени");

// Меню Установка интервала
MAKE_MENU(m_s7i1, m_s7i2, NE    , m_s1i6, NE, 0, 0, 0, "Принятие решение");
MAKE_MENU(m_s7i2, m_s7i3, m_s7i1, m_s1i6, NE, 0, 0, 0, "Мин.работы вентилятора");
MAKE_MENU(m_s7i3, m_s7i4, m_s7i2, m_s1i6, NE, 0, 0, 0, "Время аварии конд.");
MAKE_MENU(m_s7i4, m_s7i5, m_s7i3, m_s1i6, NE, 0, 0, 0, "Время сброса ШТРАФА");
MAKE_MENU(m_s7i5, NE    , m_s7i4, m_s1i6, NE, 0, 0, 0, "Записи в лог сервера");

void init_menu();
uint8_t start_menu();

#endif /*MENU_H_*/




