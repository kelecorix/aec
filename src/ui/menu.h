#ifndef MENU_H_
#define MENU_H_

typedef struct PROGMEM{
  void       *Next;
  void       *Previous;
  void       *Parent;
  void       *Child;
  uint8_t     Select;
  int min_value;
  int max_value;
  const char  Text[];
} menuItem;

#define MAKE_MENU(Name, Next, Previous, Parent, Child, Select, Text) \
  extern menuItem Next;						     \
  extern menuItem Previous;					     \
  extern menuItem Parent;					     \
  extern menuItem Child;						\
  menuItem Name = {(void*)&Next, (void*)&Previous, (void*)&Parent, (void*)&Child, (uint8_t)Select, { Text }}


menuItem	Null_Menu = {(void*)0, (void*)0, (void*)0, (void*)0, 0, {0x00}};
 
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
MAKE_MENU(m_s2i1, m_s2i2, NE    , m_s1i1, NE, 0, 0, 0, ""); 
MAKE_MENU(m_s2i2, m_s2i3, m_s2i1, m_s1i1, NE, 0, 0, 0, "");
MAKE_MENU(m_s2i3, m_s2i4, m_s2i2, m_s1i1, NE, 0, 0, 0, "");
MAKE_MENU(m_s2i4, m_s2i5, m_s2i3, m_s1i1, NE, 0, 0, 0, "");
MAKE_MENU(m_s2i5, m_s2i6, m_s2i4, m_s1i1, NE, 0, 0, 0, "");
MAKE_MENU(m_s2i6, m_s2i7, m_s2i5, m_s1i1, NE, 0, 0, 0, "");
MAKE_MENU(m_s2i7, m_s2i8, m_s2i6, m_s1i1, NE, 0, 0, 0, "");
MAKE_MENU(m_s2i8, m_s2i9, m_s2i7, m_s1i1, NE, 0, 0, 0, "");
MAKE_MENU(m_s2i9, NE    , m_s2i8, m_s1i1, NE, 0, 0, 0, "");

// Меню оборудование
MAKE_MENU(m_s3i1 , 0, 0, 0, 0, 0, 0, 0, "");
MAKE_MENU(m_s3i2 , 0, 0, 0, 0, 0, 0, 0, "");
MAKE_MENU(m_s3i3 , 0, 0, 0, 0, 0, 0, 0, "");

// Меню Лог Ошибок
MAKE_MENU(m_s4i1 , 0, 0, 0, 0, 0, 0, 0, "");
MAKE_MENU(m_s4i2 , 0, 0, 0, 0, 0, 0, 0, "");

// Меню Установка датчиков
MAKE_MENU(m_s5i1 , 0, 0, 0, 0, 0, 0, 0, "");
MAKE_MENU(m_s5i2 , 0, 0, 0, 0, 0, 0, 0, "");
MAKE_MENU(m_s5i3 , 0, 0, 0, 0, 0, 0, 0, "");

// Меню Установка датчиков - Наличие
MAKE_MENU(m_s51i1, 0, 0, 0, 0, 0, 0, 0, "");
MAKE_MENU(m_s51i2, 0, 0, 0, 0, 0, 0, 0, "");
MAKE_MENU(m_s51i3, 0, 0, 0, 0, 0, 0, 0, "");
MAKE_MENU(m_s51i4, 0, 0, 0, 0, 0, 0, 0, "");
MAKE_MENU(m_s51i5, 0, 0, 0, 0, 0, 0, 0, "");

// Меню Установка датчиков - Адреса
MAKE_MENU(m_s52i1, 0, 0, 0, 0, 0, 0, 0, "");
MAKE_MENU(m_s52i2, 0, 0, 0, 0, 0, 0, 0, "");
MAKE_MENU(m_s52i3, 0, 0, 0, 0, 0, 0, 0, "");
MAKE_MENU(m_s52i4, 0, 0, 0, 0, 0, 0, 0, "");
MAKE_MENU(m_s52i4 , 0, 0, 0, 0, 0, 0, 0, "");

// Меню Установка датчиков - Установка
MAKE_MENU(m_s53i1, 0, 0, 0, 0, 0, 0, 0, "");

// Меню Установка времени
MAKE_MENU(0 , 0, 0, 0, 0, 0, 0, 0, "");
MAKE_MENU(0 , 0, 0, 0, 0, 0, 0, 0, "");

// Меню Установка интервалоа
MAKE_MENU(0 , 0, 0, 0, 0, 0, 0, 0, "");
MAKE_MENU(0 , 0, 0, 0, 0, 0, 0, 0, "");
MAKE_MENU(0 , 0, 0, 0, 0, 0, 0, 0, "");
MAKE_MENU(0 , 0, 0, 0, 0, 0, 0, 0, "");
MAKE_MENU(0 , 0, 0, 0, 0, 0, 0, 0, "");

#endif /*MENU_H_*/




