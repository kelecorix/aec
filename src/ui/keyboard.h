#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#define KEY_UP     1
#define KEY_DOWN   2
#define KEY_RIGHT  3
#define KEY_LEFT   4

typedef struct KB {

  Site* site;
  int fd;      // дискриптор для работы с шиной
  int address;
  int connect; // // -1 = unknown, 0 = not connected, 1 = connected

} KB;

#endif /* KEYBOARD_H_ */
