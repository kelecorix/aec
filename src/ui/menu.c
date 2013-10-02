#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#include "lcd.h"
#include "keyboard.h"
#include "menu.h"
#include "../hw/site.h"
#include "../config/config.h"

int mode;                   // режим редактирования или нет
Menu* menu;

void init_menu() {
  create_menu();
}


void create_menu(){

  menu = malloc(sizeof(Menu));

  create_node(1, 0, 0, 0, "Температуры", "");
  create_node(2, 0, 0, 0, "Оборудование", "");
  create_node(3, 0, 0, 0, "Лог Ошибок", "");
  create_node(4, 0, 0, 0, "Установка датчиков", "");
  create_node(5, 0, 0, 0, "Установка времени", "");
  create_node(6, 0, 0, 0, "Установка интервалов", "");

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

  // Меню оборудование
  create_node(21, 2, 0, 1, "Миксер", "is_mix");  // Да/НЕТ
  create_node(22, 2, 0, 1, "ТЭН", "is_ten");     // Да/НЕТ
  create_node(23, 2, 0, 2, "Кол. кондиционеров", "num_ac");

  // Меню Лог Ошибок
  create_node(31, 3, 0, 0, "Просмотреть", "");
  create_node(32, 3, 0, 0, "Очистить", "");

  // Меню Установка датчиков
  create_node(41, 4, 0, 0, "Наличие", "");
  create_node(42, 4, 0, 0, "Адреса", "");
  create_node(43, 4, 0, 0, "Установка", "");

  // Меню Установка датчиков - Наличие
  create_node(411, 4, 0, 0, "Улица", "");
  create_node(412, 4, 0, 0, "Сайт", "");
  create_node(413, 4, 0, 0, "Миксер", "");
  create_node(414, 4, 0, 0, "Кондиционер1", "");
  create_node(415, 4, 0, 0, "Кондиционер2", "");

  // Меню Установка датчиков - Адреса
  create_node(421, 4, 0, 0, "Улица", "");
  create_node(422, 4, 0, 0, "Сайт", "");
  create_node(423, 4, 0, 0, "Миксер", "");
  create_node(424, 4, 0, 0, "Кондиционер1", "");
  create_node(425, 4, 0, 0, "Кондиционер2", "");

  // Меню Установка датчиков - Установка
  create_node(431, 4, 0, 0, "Установка", "");

  // Меню Установка времени
  create_node(51, 5, 0, 0, "Установка даты", "");
  create_node(52, 5, 0, 0, "Установка времени", "");

  // Меню Установка интервала
  create_node(61, 6, 0, 0, "Принятие решение", "");
  create_node(62, 6, 0, 0, "Мин.работы вентилятора", "");
  create_node(63, 6, 0, 0, "Время аварии конд.", "");
  create_node(64, 6, 0, 0, "Время сброса ШТРАФА", "");
  create_node(65, 6, 0, 0, "Записи в лог сервера", "");

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

  Node* node = malloc(sizeof(Node));

  node->id     = id;
  node->min    = min;
  node->max    = max;
  node->text   = text;
  node->cn     = cn;
  node->parent = get_parent_by_id(parent);
  node->val    = strtol(getStr(site->cfg, (void *) cn), (char **) NULL, 10);

  // В конце
  if (id == 0) {
    menu->root = node;
    menu->curr = node;
  }
  menu->length++;
  menu->nodes[menu->length] = node;
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

  node->lenght++;
  node->parent->childs = calloc((node->lenght), sizeof(Node*));
  node->parent->childs[node->lenght] = node;

}

Node* get_parent(Node* node){

  return NULL;

}

Node* get_parent_by_id(int id){

  return NULL;
}

Node* next_child(Node* node){
  return NULL;
}

Node* prev_child(Node* node){
  return NULL;

}

// аргумент узел на котором мы находимся
// возвр: первый узел следующего уровня
Node* next_level(Node* node){
  return NULL;
}

// аргумент узел котором мы находимся
// возвр: первый узел предыдущего уровня
Node* prev_level(Node* node){
  return NULL;
}

// обойти дерево
void traverse(){

}

