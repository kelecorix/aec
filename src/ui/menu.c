#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>

#include "lcd.h"
#include "keyboard.h"
#include "../utils/utils.h"
#include "../hw/site.h"
#include "../config/config.h"
#include "menu.h"

int mnmode; // режим меню или вывод экрана на дисплей
int emode;  // режим редактирования
int smode;  // спец режим: 1,2,3,4 - режим лога, режим уст. датчиков, режим уст. даты, режим уст. времени
int mval;
int pos;   // текущая позиция от 1 до 3
int lpos;  // текущая позиция лога
int chld;  // id текущего нода
int entr;
int nempty;
int omode; // Режим вывода
int pre_pos;
int pre_chld;

char *lines[3]; // массив полных линий для лога
int lmc;

Menu* menu;
OutNode** outs;

void init_menu() {
  create_menu();
}

void create_menu() {

  printf("предварительная организация\n");

  int i;
  menu = malloc(sizeof(Menu));
  menu->length = 47;
  menu->nodes = (Node**) malloc(47 * sizeof(Node*));

  for (i = 0; i < 47; i++) {
    menu->nodes[i] = (Node*) malloc(sizeof(Node));
  }

  mnmode = 0;
  pos = -1;
  chld = 0;
  entr = 0;

  // Последняя запись может соответствовать
  // "" - узел перехода
  // "xxxxxxx_xxxx" - пункт конфигурации
  // "v" -
  // "x" -
  // "i" -
  // ""

  // Корневой узел
  create_node(0, 0, 0, 0, "Меню", "", NULL);

  // Главное меню
  create_node(1, 0, 0, 0, "Температуры"    , "", NULL);
  create_node(2, 0, 0, 0, "Оборудование"   , "", NULL);
  create_node(3, 0, 0, 0, "Лог Ошибок"     , "", NULL);
  create_node(4, 0, 0, 0, "Уст. датчиков"  , "", NULL);
  create_node(5, 0, 0, 0, "Уст. времени"   , "", NULL);
  create_node(6, 0, 0, 0, "Уст. интервалов", "", NULL);

  // Температуры
  create_node(11, 1, 10, 40, "Поддержания"  ,"temp_support", NULL);
  create_node(12, 1, 1, 10, "Дельта поддерж", "diff", NULL);
  create_node(13, 1, -10, 20, "Догрева"     , "temp_heat", NULL);
  create_node(14, 1, -10, 20, "Росы улица"  , "temp_dew", NULL);
  create_node(15, 1, -10, 20, "Миксер"      , "temp_mix", NULL);
  create_node(16, 1, 0, 80, "Аварии"        , "temp_fail", NULL);
  create_node(17, 1, 0, 20, "Дельта окруж"  , "diff_out", NULL);
  create_node(18, 1, 0, 30, "Дельта конд 1" , "diff_ac_1", NULL);
  create_node(19, 1, 0, 30, "Дельта конд 2" , "diff_ac_2", NULL);

  // Оборудование
  create_node(20, 2, 0, 1, "Миксер"   , "is_mix", NULL);  // Да/НЕТ
  create_node(21, 2, 0, 1, "ТЭН"      , "is_ten", NULL);     // Да/НЕТ
  create_node(22, 2, 0, 2, "Кол. конд", "num_ac", NULL);

  // Лог Ошибок
  create_node(23, 3, 0, 0, "Просмотреть", "l" , NULL); // mval -100
  create_node(24, 3, 0, 0, "Очистить"   , "cl", NULL);   // mval -200

  // Меню Установка датчиков
  create_node(25, 4, 0, 0, "Наличие"  , "", NULL);
  create_node(26, 5, 0, 0, "Адреса"   , "", NULL);
  create_node(27, 4, 0, 0, "Установка", "", NULL);

  // Установка датчиков - Наличие
  create_node(28, 25, 0, 1, "Улица"  , "", &site->sens_out);
  create_node(29, 25, 0, 1, "Сайт"   , "", &site->sens_in);
  create_node(30, 25, 0, 1, "Миксер" , "", &site->sens_mix);
  create_node(31, 25, 0, 1, "Конд1"  , "", &site->sens_ac1);
  create_node(32, 25, 0, 1, "Конд2"  , "", &site->sens_ac2);

  // Установка датчиков - Адреса
  create_node(33, 26, 0, 0, "Улица" , "s_temp_outdoor", NULL);
  create_node(34, 26, 0, 0, "Сайт"  , "s_temp_indoor" , NULL);
  create_node(35, 26, 0, 0, "Миксер", "s_temp_mix"    , NULL);
  create_node(36, 26, 0, 0, "Конд1" , "s_temp_evapor1", NULL);
  create_node(37, 26, 0, 0, "Конд2" , "s_temp_evapor2", NULL);

  // Установка датчиков - Установка
  create_node(38, 27, 0, 0, "Установка", "i", NULL); //mval -300

  // Меню Установка времени
  create_node(39, 5, 0, 0, "Уст. даты", "d", NULL);  //mval -400
  create_node(40, 5, 0, 0, "Уст. времени", "t", NULL); //mvak -500

  // Меню Установка интервала
  create_node(41, 6, 0, 0, "Принятие решение"    , "time_decision"     , NULL);
  create_node(42, 6, 0, 0, "Мин.раб вент"        , "time_vent_min_work", NULL);
  create_node(43, 6, 0, 0, "Время аварии конд."  , "time_fail_ac"      , NULL);
  create_node(44, 6, 0, 0, "Время сброса ШТРАФА" , "time_penalty"      , NULL);
  create_node(45, 6, 0, 0, "Записи в лог сервера", "time_log"          , NULL);

  menu->root = menu->nodes[0];
  menu->curr = menu->nodes[0];

}

Node* get_parent_by_id(int id) {
  return menu->nodes[id];
}

void add_child_to_parent(Node* parent, Node* node) {

  if (node->id == 0)
    return; //это корневой узел

  parent->childs[parent->lenght] = node;
  printf("родитель %d, потомка %d,c длиной %d \n", parent->id, node->id, parent->lenght);
  parent->childs[parent->lenght] = node;
  printf("text добавленного: %s\n", parent->childs[parent->lenght]->text);
  parent->lenght++;

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
void create_node(int id, int parent, int min, int max, char* ctext, char* cn, void* param) {

  printf("Cоздадим нод\n");

  Node* node = malloc(sizeof(Node));
  int i;
  node->id = id;
  node->min = min;
  node->max = max;
  node->text = ctext;
  node->cn = cn;
  node->lenght = 0;
  node->ptr_param = param;
  node->childs = malloc(9 * sizeof(Node*));
  for (i = 0; i < 9; i++) {
    node->childs[i] = malloc(sizeof(Node));
  }

  if ((strcmp(node->cn, "") != 0) && (strcmp(node->cn, "l") != 0) && (strcmp(node->cn, "cl") != 0) &&
      (strcmp(node->cn, "i") != 0) && (strcmp(node->cn, "d") != 0) && (strcmp(node->cn, "t") != 0))
    node->val = strtol(getStr(site->cfg, (void *) cn), (char **) NULL, 10);

//
  if (node->id > 0)
    node->parent = get_parent_by_id(parent);

  add_child_to_parent(node->parent, node);

  menu->nodes[id] = node;
}

int getDepth(Menu* tree, Node* p) {
  if (isRoot(p))
    return 0;
  else
    return 1 + getDepth(tree, p->parent);
}

int getHeight(Menu* tree) {
  Node* node;
  int i, h = 0;

  for (i = 0; i < menu->length; i++) {
    node = tree->nodes[i];
    if (isLeaf(node))
      h = MAX(h, getDepth(tree, node));
  }

  return h;
}

int isRoot(Node* node) {
  if (node->parent == NULL)
    return 1;
  else
    return 0;
}

int isLeaf(Node* node) {
  if (node->lenght == 0)
    return 1;
  else
    return 0;
}

Node* get_parent(Node* node) {

  return NULL;

}

Node* next_child(Node* parent) {
  return parent;
}

Node* prev_child(Node* node) {

  return NULL;
}

Node* next_level(Node* node) {

  int i;
  Node* n;

  for (i = node->id; i < menu->length; i++) {
    if (isLeaf(menu->nodes[i]))
      continue;
    else {
      n = menu->nodes[i];
      break;
    }
  }
  return n;
}

// аргумент узел котором мы находимся
// возвр: первый узел предыдущего уровня
Node* prev_level(Node* node) {
  if (!isRoot(menu->curr))
    return node->parent;
  else {
    mnmode = 0;
    return menu->curr;
  }
}

// обойти дерево
void traverse() {

}

void disp(Disp* lcd){

  char tmp_time[50], tmp_temp_out[50], tmp_temp_in[50], tmp_temp_mix[50], tmp_temp_evapor1[50], tmp_temp_evapor2[50];

  time_t rawtime;
  struct tm * timeinfo;
  //printf("режим %d\n", omode);

  if (omode == 1) {
    // экран 1 - темп сайта , улицы
    //printf("экран1\n");
    time(&rawtime);
    //printf("время\n");
    timeinfo = localtime(&rawtime);
    //printf("сделали структуру\n");
    sprintf(tmp_time, "%2d/%02d  %2d:%02d:%02d", timeinfo->tm_mday, 1 + timeinfo->tm_mon, timeinfo->tm_hour, timeinfo->tm_min,
        timeinfo->tm_sec);

    if (site->temp_out != -100.0) {
      sprintf(tmp_temp_out, "Улица  = %2.2f°C", site->temp_out);
    } else {
      sprintf(tmp_temp_out, "Улица  =  Ошибка");
    }
    //printf("2\n");
    if (site->temp_in != -100.0) {
      sprintf(tmp_temp_in, "Сайт   = %2.2f°C", site->temp_in);
    } else {
      sprintf(tmp_temp_in, "Сайт   =  Ошибка");
    }
    //printf("3\n");
    //printf("0\n");
    lcd_line(lcd, tmp_temp_out, 0);
    //printf("1 %s \n", tmp_temp_in);
    lcd_line(lcd, tmp_temp_in, 1);
    //printf("2\n");
    lcd_line(lcd, "Состояние работы", 2);
    //printf("3\n");
    lcd_line(lcd, tmp_time, 3);

    //printf("экран1 конец\n");
  }

  //экран 2 - конд, конд, миксер, время
  if (omode == 2) {
    printf("экран2\n");
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    sprintf(tmp_time, "%2d/%02d  %2d:%02d:%02d", timeinfo->tm_mday, 1 + timeinfo->tm_mon, timeinfo->tm_hour, timeinfo->tm_min,
        timeinfo->tm_sec);

    if (site->temp_evapor1 != -100.0) {
      sprintf(tmp_temp_evapor1, "Конд1  = %2.2f°C", site->temp_evapor1);
    } else {
      sprintf(tmp_temp_evapor1, "Конд1  =  Ошибка");
    }

    if (site->temp_evapor2 != -100.0) {
      sprintf(tmp_temp_evapor2, "Конд2  = %2.2f°C", site->temp_evapor2);
    } else {
      sprintf(tmp_temp_evapor2, "Конд2  =  Ошибка");
    }

    if (site->temp_mix != -100.0) {
      sprintf(tmp_temp_mix, "Миксер = %2.2f°C", site->temp_mix);
    } else {
      sprintf(tmp_temp_mix, "Миксер =  Ошибка");
    }
    lcd_line(lcd, tmp_temp_evapor1, 0);
    lcd_line(lcd, tmp_temp_evapor2, 1);
    lcd_line(lcd, tmp_temp_mix, 2);
    lcd_line(lcd, tmp_time, 3);
    printf("экран2 конец\n");
  }

  if (omode == 3) {

    //экран - обороты вент, вент2, заслонка полож

  }
}

// for branches
void disp_item(Disp* lcd) {

  if (isLeaf(menu->curr))
    return;

  reset(lcd);
  int i;

  char *z, *out;
  lcd_line(lcd, menu->curr->text, 0);

  for (i = 0; i < 3; i++) {

    if (i == pos) {
      z = ">";
    } else
      z = " ";

    if ((chld + i) < menu->curr->lenght) {
      out = concat(z, menu->curr->childs[chld + i]->text);
      lcd_line(lcd, out, i + 1);
      nempty = 0;
    } else {
      lcd_line(lcd, "                ", i + 1);
      nempty = i + 1;
    }
  }

  entr++;

  usleep(50000);
}

// for leafs
void disp_item_edit(Disp* lcd, int num) {

  printf("подготовим вывод\n");
  // i2c write на основани
  reset(lcd);
  printf("подготовим вывод 2 \n");
  char buf[100];
  sprintf(buf, "%d", num);

  lcd_line(lcd, concat(menu->curr->childs[chld + pos]->text, ":"), 0);
  lcd_line(lcd, "       ^     ", 1);
  lcd_line(lcd, concat("       ", buf), 2);
  lcd_line(lcd, "       v     ", 3);

}

void disp_log(Disp* lcd){

  printf("режим вывода лога\n");

  int cpos;

  char *buf[16]; // буфер вывода

  reset(lcd);
  int i, j, rd;
  char *z, *out;

  FILE *fp;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  fp = fopen(concat(gcfg->ldir, "event.log"), "r");
  if (fp == NULL)
    exit(EXIT_FAILURE);

  lcd_line(lcd, "Лог: ", 0); // Всегда стоит наверху

  for(i=0, j=0; i<lpos+3; i++){
    getline(&line, &len, fp);
    if (i>=lpos){
      lines[j]=line;
      memcpy(buf, line, 16);
      j++;
      lcd_line(lcd, buf, j);
    }
  }
  fclose(fp);
}

void disp_log_move(Disp* lcd, int direct){

  // direct 0 - left
  // direct 1 - right
  int i;
  char *buf[16];
  for(i=0;i<3;i++){
    if(direct==0)
      memcpy(buf, lines[i]-(lmc*16), 16);
    else
      memcpy(buf, lines[i]+(lmc*16), 16);
    lcd_line(lcd, buf, i);
  }

}

/*  Установка датчиков
 *
 */
void disp_sens(Disp* lcd){

  int i, j, len;
  char *tokens[12];
  // прочитаем датчики доступные на плате
  len = list_sensors(tokens);

  // попробуем найти эти датчики в конфиге




  lcd_line(lcd, "Лог: ", 0); // Всегда стоит наверху
//
//  for(i=0, j=0; i<lpos+3; i++){
//    getline(&line, &len, fp);
//    if (i>=lpos){
//      j++;
//      lcd_line(lcd, line, j);
//      }
//   }
//



  get_data_by_name(gcfg->conn, tokens[i]);




}

/*  Изменение конкретного датчика
 *
 */
void disp_sens_edit(Disp* lcd){


}

/*  Установка даты
 *
 */
void disp_date(Disp* lcd){


}
/*  Установка времени
 *
 */
void disp_time(Disp* lcd){


}

void clear_log(){
  remove(concat(gcfg->ldir, "event.log"));
}

void save_value(){
  char cval[20];
  sprintf(cval, "%d", mval);
  hashmapPut(site->cfg->mTable, menu->curr->cn, cval);
  writeConfig(concat(gcfg->cdir, "freecooling.conf"));
  menu->curr->childs[chld + pos]->val = mval;
}

void onKeyClicked(Disp* lcd, int key_code) {
  printf("нажата кнопка %d\n", key_code);
  switch (key_code){
  case KEY_LEFT :

    printf("LEFT %d %d %d %d\n", chld, pos, entr, nempty);

    if(emode==1){
      emode=0;
      disp_item(lcd);
      break;
    }

    if(smode==1){
      disp_log_move(lcd, 0);
    }

    if (mnmode == 0)
      break;

    printf("id %d\n", menu->curr->id);
    if (mnmode == 1 ){
      if(menu->curr->id > 0){
        menu->curr = menu->curr->parent;
        pos=0;
      }  else{
        mnmode=0;
        pos=-1;
        chld=0;
        disp(lcd);
        break;
      }
    }

    if (isLeaf(menu->curr)) {
      emode = 1;
      mval = menu->curr->childs[chld + pos]->val;
      disp_item_edit(lcd, mval);
      break;
    }

    disp_item(lcd);
    break;
  case KEY_RIGHT :

    if(emode==1)
      break;

    if(smode==1){
      disp_log_move(lcd, 1);
    }

    if (mnmode == 0) {
      mnmode = 1;
      chld = 0;
      pos++;
      //menu->curr = menu->curr->childs[chld];
      disp_item(lcd);
      break;
    }

    if (mnmode == 1) {
      // перейдем на уровень вниз
      // если это не лист
      // установим позицию на первом меню
      if (!isLeaf(menu->curr->childs[chld + pos])) {
        printf("RIGHT %d %d %d %d\n", chld, pos, entr, nempty);
        menu->curr = menu->curr->childs[chld + pos];
        pos=0; // для того, чтоб попадать на первый пункт меню
        disp_item(lcd);
        break;
      }else{
        select_item(lcd);
      }
    }
    break;
  case KEY_UP :

    if (emode == 1) {
      // режим редактирования значения
      mval++;
      disp_item_edit(lcd, mval);
      break;
    }

    if(smode==1){
      //в режиме просмотра лога
      lpos=lpos+3;
      disp_log(lcd);
    }

    if (mnmode == 1) {

      if (pos == 0) {
        chld = chld - 3;
        entr = 0;
        pos = 2;
      } else
        pos--;

      if (chld == -3) { // Защита от дурака
        chld = 0;
        pos = 0;
      }
      //break;

      printf("UP %d %d %d %d\n", chld, pos, entr, nempty);

      disp_item(lcd);

      break;
    }

    if (mnmode == 0) {
      // находимся в режиме вывода
      // изменим отображение экрана

    }

    break;
  case KEY_DOWN :

    if (emode == 1) {
      // режим редактирования значения
      mval--;
      disp_item_edit(lcd, mval);
      break;
    }

    if(smode==1){
      //в режиме просмотра лога
      lpos=lpos+3;
      disp_log(lcd);
    }

    if (mnmode == 1) {

      // 3 количество позиций для вывода
      printf("DOWN %d %d %d %d\n", chld, pos, entr, nempty);
      if ((nempty - pos >= 2) || ((pos == 2) && ((chld+3)>=menu->curr->lenght)))     //защита от дурака, след.строка пустая
        break;

      pos++;

      if (pos == 3) {
        chld = chld + 3;
        entr = 0;
        pos = 0;
      }

      disp_item(lcd);
      break;
    }

    break;
  case KEY_OK :
    //если это лист, тогда перейдем в редактирование
    //перед if
    if(emode==1){
      // мы нажали ОК в режиме редактирования, значит нужно
      // сохранить значение
      save_value();
      // TODO: вернуться в предыдущее меню
      disp_item(lcd);
    }

    printf("Меню - %s %d\n", menu->curr->childs[chld + pos]->text, menu->curr->childs[chld + pos]->lenght);
    if (isLeaf(menu->curr->childs[chld + pos])) {
      printf("в ифе\n");
      select_item(lcd);
    }
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
    //printf("Перед чтением\n");
    if (read(kb->fd, buf, 1) != 1) {
      printf("BUTTONS Error reading from i2c\n");
      kb->connect = 0;
    } else {
      //printf("значение %d\n", buf[0]);
      key = (uint) buf[0];
      return key;
    }
  }

  return key;
}

void change_value(int direct) {

  if ((mval == menu->curr->min) || (mval == menu->curr->min))
    return; // граничные значения

  if (direct == 0)
    mval--;
  else
    mval++;
}

void select_item(Disp* lcd) {
  printf("установим режим выбора\n");

  // Переходим в специфические режимы
  //
  char *nm = menu->curr->childs[chld + pos]->cn;

  if (strcmp(nm, "l")==0){
    //показ лога
    lpos=0;
    smode=1;
    disp_log(lcd);
    return;
  }

  if (strcmp(nm, "cl")==0){
    //очистка лога
    clear_log();
    return;
  }

  if (strcmp(nm, "i")==0){
   //установка датчиков
    lpos=0;
    smode=2;
    disp_sens(lcd);
    return;
  }

  if (strcmp(nm, "d")==0){
   //установка даты
    smode=3;
    disp_date(lcd);
    return;
  }

  if (strcmp(nm, "t")==0){
    // установка времени
    smode=4;
    disp_time(lcd);
    return;
  }

  if (emode == 1) {
    // сохраняем значения выходим обратно
    emode = 0;
    disp_item(lcd);
  } else {
    emode = 1;
    mval = menu->curr->childs[chld + pos]->val;
    disp_item_edit(lcd, mval);
  }
}
