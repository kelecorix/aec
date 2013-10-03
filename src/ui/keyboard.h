#ifndef KEYBOARD_H_
#define KEYBOARD_H_

typedef struct KB {

  int fd;      // дискриптор для работы с шиной
  int address;
  int connect; // -1 = unknown, 0 = not connected, 1 = connected

} KB;

KB* kb_new(int addr);

#endif /* KEYBOARD_H_ */
