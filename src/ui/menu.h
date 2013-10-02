#ifndef MENU_H_
#define MENU_H_

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "lcd.h"
#include "keyboard.h"
#include "../hw/site.h"
#include "../config/config.h"

#define KEY_LEFT  127
#define KEY_RIGHT 447
#define KEY_UP    239
#define KEY_DOWN  191
#define KEY_OK    223

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

/*
 *
 *
 *
 */
typedef struct Node{
  int          id;
  char*        text;
  char*        cn;  // config name
  int          min;
  int          max;
  int          val;
  struct Node* parent;
  struct Node** childs;
  int          lenght; // childs lenght
} Node;

/*
 *
 *
 *
 */
typedef struct Tree{
  int   length; //of nodes
  int   depth;
  int   height;
  Node* root;
  Node* curr;
  Node** nodes;
} Menu;

void init_menu();
void create_menu();
void create_node(int id, int parent, int min, int max, char* text, char* cn);
void traverse();
void add_node(Node* node);
void add_child(Node* node);
Node* get_parent(Node* node);
Node* get_parent_by_id(int id);
Node* next_child(Node* node);
Node* prev_child(Node* node);
Node* next_level(Node* node);
Node* prev_level(Node* node);
int isRoot(Node* node);
int isLeaf(Node* node);
int getDepth(Menu* menu, Node* p);
int getHeight(Menu* menu);

#endif /*MENU_H_*/




