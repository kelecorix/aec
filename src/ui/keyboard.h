#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#define KEY_LEFT  127
#define KEY_RIGHT 447
#define KEY_UP    239
#define KEY_DOWN  191
#define KEY_OK    223

typedef struct KB {

  int fd;      // дискриптор для работы с шиной
  int address;
  int connect; // -1 = unknown, 0 = not connected, 1 = connected

} KB;

KB* kb_new(int addr);
void reset_kb(KB* kb);

#endif /* KEYBOARD_H_ */
