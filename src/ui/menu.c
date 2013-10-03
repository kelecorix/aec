#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#include "lcd.h"
#include "keyboard.h"
#include "menu.h"
#include "../hw/site.h"
#include "../config/config.h"

int mnmode;                   // режим редактирования или нет
int mval;
Menu* menu;

void init_menu() {
  create_menu();
}


void create_menu(){

  int i;
  printf("предварительная организация\n");
  menu = malloc(sizeof(Menu));
  menu->length = 47;
  menu->nodes = calloc(menu->length, sizeof(Node*));
  for(i=0; i<menu->length; i++){
    menu->nodes[i] = malloc(sizeof(Node*));
  }

  mnmode = 0;
  printf("Начинаем создавать пункты\n");
  create_node(0, 0, 0, 0, "Меню", ""); // корневой узел
  printf("Первый блок\n");
  create_node(1, 0, 0, 0, "Температуры", "");
  create_node(2, 0, 0, 0, "Оборудование", "");
  create_node(3, 0, 0, 0, "Лог Ошибок", "");
  create_node(4, 0, 0, 0, "Установка датчиков", "");
  create_node(5, 0, 0, 0, "Установка времени", "");
  create_node(6, 0, 0, 0, "Установка интервалов", "");
  printf("Второй блок\n");
  // Меню температуры
  create_node(11, 1, 10, 40, "Поддержания", "temp_support");
  create_node(12, 1,  1, 10, "Дельта поддержания", "diff");
  create_node(13, 1,-10, 20, "Догрева", "temp_heat");
  create_node(14, 1,-10, 20, "Росы улица", "temp_dew");
  create_node(15, 1,-10, 20, "Миксер", "temp_mix");
  create_node(16, 1,  0, 80, "Аварии", "temp_fail");
  create_node(17, 1,  0, 20, "Дельта окружающей", "diff_out");
  create_node(18, 1,  0, 30, "Дельта конд 1", "diff_ac_1");
  create_node(19, 1,  0, 30, "Дельта конд 2", "diff_ac_2");
  printf("Третий блок\n");
  // Меню оборудование
  create_node(20, 2, 0, 1, "Миксер", "is_mix");  // Да/НЕТ
  create_node(21, 2, 0, 1, "ТЭН", "is_ten");     // Да/НЕТ
  create_node(22, 2, 0, 2, "Кол. кондиционеров", "num_ac");
  printf("Чертвертый блок\n");
  // Меню Лог Ошибок
  create_node(23, 3, 0, 0, "Просмотреть", "");
  create_node(24, 3, 0, 0, "Очистить", "");
  printf("Пятый блок блок\n");
  // Меню Установка датчиков
  create_node(25, 4, 0, 0, "Наличие", "");
  create_node(26, 4, 0, 0, "Адреса", "");
  create_node(27, 4, 0, 0, "Установка", "");
  printf("Шестой блок\n");
  // Меню Установка датчиков - Наличие
  create_node(28, 4, 0, 0, "Улица", "");
  create_node(29, 4, 0, 0, "Сайт", "");
  create_node(30, 4, 0, 0, "Миксер", "");
  create_node(31, 4, 0, 0, "Кондиционер1", "");
  create_node(32, 4, 0, 0, "Кондиционер2", "");
  printf("Седьмой блок\n");
  // Меню Установка датчиков - Адреса
  create_node(33, 4, 0, 0, "Улица", "");
  create_node(34, 4, 0, 0, "Сайт", "");
  create_node(35, 4, 0, 0, "Миксер", "");
  create_node(36, 4, 0, 0, "Кондиционер1", "");
  create_node(37, 4, 0, 0, "Кондиционер2", "");
  printf("Восьмой блок\n");
  // Меню Установка датчиков - Установка
  create_node(38, 4, 0, 0, "Установка", "");
  printf("Девятый блок\n");
  // Меню Установка времени
  create_node(39, 5, 0, 0, "Установка даты", "");
  create_node(40, 5, 0, 0, "Установка времени", "");
  printf("Десятый блок\n");
  // Меню Установка интервала
  create_node(41, 6, 0, 0, "Принятие решение", "");
  create_node(42, 6, 0, 0, "Мин.работы вентилятора", "");
  create_node(43, 6, 0, 0, "Время аварии конд.", "");
  create_node(44, 6, 0, 0, "Время сброса ШТРАФА", "");
  create_node(45, 6, 0, 0, "Записи в лог сервера", "");
  printf("Завершили создание нодов \n");
}

/* Функция создания узла дерева меню
 *
 *  Параметры:
 *            id     - уникальный номер узла
 *            parent - уникальный номер родителя
 *            min    - минимальное значение
 *            max    - максимальное значение
 *            text   - текст меню
 *            ct     - имя параметра в конфиге, если есть
 */
void create_node(int id, int parent, int min, int max, char* text, char* cn){

  printf("CСоздадим нод\n");

  Node* node = menu->nodes[id];

  node->id     = id;
  node->min    = min;
  node->max    = max;
  node->text   = text;
  node->cn     = cn;
  if(node->id != 0)
    node->parent = get_parent_by_id(parent);
  if (strcmp(node->cn, "") != 0)
    node->val = strtol(getStr(site->cfg, (void *) cn), (char **) NULL, 10);

  // В конце
  if (id == 0) {
    menu->root = node;
    menu->curr = node;
  }

  add_child(node);
}

int getDepth(Menu* tree, Node* p){
  if(isRoot(p))
    return 0;
  else
    return 1 + getDepth(tree, p->parent);
}

int getHeight(Menu* tree){
  Node* node;
  int i, h = 0;

  for(i=0; i< menu->length; i++){
    node = tree->nodes[i];
    if(isLeaf(node))
      h = MAX(h, getDepth(tree, node));
  }

  return h;
}

int isRoot(Node* node){
  if (node->parent == NULL)
    return 1;
  else
    return 0;
}

int isLeaf(Node* node){
  if(node->childs == NULL)
    return 1;
  else
    return 0;
}

void add_child(Node* node){

  printf("Добавим потомков \n");

  if (node->id == 0)
    return; //это корневой узел

  node->lenght++;
  node->parent->childs = calloc((node->lenght), sizeof(Node));
  node->parent->childs[node->lenght-1] = node;
}

Node* get_parent(Node* node){

  return NULL;

}

Node* get_parent_by_id(int id){
  return menu->nodes[id];
}

Node* next_child(Node* node){
  return NULL;
}

Node* prev_child(Node* node){

  return NULL;
}

Node* next_level(Node* node){

  int i;
  Node* n;

  for(i=node->id; i<menu->length; i++){
    if(isLeaf(menu->nodes[i]))
      continue;
    else{
      n=menu->nodes[i];
      break;
    }
  }

  return n;
}

// аргумент узел котором мы находимся
// возвр: первый узел предыдущего уровня
Node* prev_level(Node* node){
  if(!isRoot(menu->curr))
    return node->parent;
  else{
    mnmode =0;
    return menu->curr;
  }
}

// обойти дерево
void traverse(){

}

void onKeyClicked(Disp* lcd, int key_code) {
  printf("нажата кнопка %d", key_code);
  switch (key_code){
  case KEY_LEFT :
    menu->curr = prev_level(menu->curr);
    disp_item(lcd);
    break;
  case KEY_RIGHT :
    //TODO: проверить или это первый вход в меню
    // когда доходим до полследнего возвр к первому
    menu->curr = next_child(menu->curr);
    disp_item(lcd);
    break;
  case KEY_UP :
    if (mnmode == 1) {
      mval = menu->curr->val;
      change_value(1);
      disp_item_edit();
    } else {
      menu->curr = prev_child(menu->curr);
      disp_item(lcd);
    }
    break;
  case KEY_DOWN :
    if (mnmode == 1) {
      mval = menu->curr->val;
      change_value(0);
      disp_item_edit();
    } else {
      menu->curr = next_child(menu->curr);
      disp_item(lcd);
    }
    break;
  case KEY_OK :
    select_item();
    disp_item_edit(lcd);
    break;
  }
}

int readKeys(KB* kb) {

  //printf("считаем нажатие\n");
  int key = 0;
  char buf[1];
  //printf("есть клавиатура? %d\n", kb->connect);
  if (kb->connect == -1) {
    reset_kb(kb);
  }
  if (kb->connect == 0) {
    return key;
  }

  if (kb->connect == 1) {
   // printf("Перед чтением\n");
    if (read(kb->fd, buf, 1) != 1) {
      printf("BUTTONS Error reading from i2c\n");
      kb->connect = 0;
    } else {
     // printf("значение %d\n", buf[0]);
      key = (uint) buf[0];
      return key;
    }
  }

  return key;
}

void disp_item(Disp* lcd){

  reset(lcd);
  mnmode = 1;

  lcd_line(lcd, "     ^     "   , 0);
  lcd_line(lcd, menu->curr->text, 1);
  lcd_line(lcd, menu->curr->val , 2);
  lcd_line(lcd, "     v     "   , 3);

}

void disp_item_edit(Disp* lcd){

  // i2c write на основани

  // mval

}

void change_value(int direct){

  if((mval==menu->curr->min) || (mval==menu->curr->min))
    return; // граничные значения

  if (direct == 0)
    mval--;
  else
    mval++;
}


void select_item(){

  if (mnmode==1)
    mnmode = 0;
  else
    mnmode = 1;

}
