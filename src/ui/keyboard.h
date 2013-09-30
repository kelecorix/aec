#ifndef KEYBOARD_H_
#define KEYBOARD_H_

typedef struct KB {

  Site* site;
  int fd;      // дискриптор для работы с шиной
  int address;
  int connect; // -1 = unknown, 0 = not connected, 1 = connected

} KB;

#endif /* KEYBOARD_H_ */
