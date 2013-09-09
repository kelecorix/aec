#include <string.h>
#include <ownetapi.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ow.h"
#include "../hw/site.h"
#include "../utils/utils.h"
#include "../config/config.h"

OWNET_HANDLE create_server_conn(char *addr) {

  OWNET_HANDLE owh;

  owh = OWNET_init(addr);
  if (owh < 0) {
    printf("OWNET_init(%s) failed.\n", addr); //TODO: писать в лог о проблемах
  }

  return owh;

}

int close_server_conn(OWNET_HANDLE conn) {

  OWNET_close(conn);
  return 1;
}

float get_data(OWNET_HANDLE conn, char* filename, int lim) {

  char *data = NULL;
  char *temp = "/temperature";

  char *full_path = concat(filename, temp);

  if (OWNET_present(conn, full_path) == 0) {

    int length = OWNET_read(conn, full_path, &data);

    if (length < 0) {
      // произошла ошибка доступа к серверу
      printf("OWNET_read error!\n");
      return -100.f;
    }

    // Считываение прошло нормально

    if (data == " ") { //компилятор ругается
      // пытаемся считать еще несколько раз
      int i;
      for (i=0; i<lim; i++){
        OWNET_read(conn, full_path, &data);
        if (data != " ")
          break;
      }

      if (data == " "){
        // все еще пусто после повторных опросов
        return -100.f; // вернем ошибку
      }

    }

    // уберем лидирующие пробелы
    char *datat = ltrim(data);
    float d = atof(datat);

    // Проверим или данные входят в рабочий диапазон
    // -55 .. +125
    // TODO: можно ли получить эти данные программно?
    if (d > -55.f && d < 125.f)
      return d;
    else
      return -100.f;

  } else
    return -100.f;

}

int list_sensors(Site* site, char *tokens[]) {

  char *dirs = NULL;
  int res = OWNET_dirlist(site->conn, site->mount_point, dirs);
  int ret = splitString(dirs, tokens, ',');

  return 0;

}

int read_sensors(Site* site) {

  OWNET_HANDLE conn;
  conn = site->conn;

  char *mnt = site->mount_point;

  char *key = "s_temp_outdoor";
  char *s_temp_out = getStr(site->cfg, (void *) key);

  key = "s_temp_indoor";
  char *s_temp_in = getStr(site->cfg, (void *) key);

  key = "s_temp_mix";
  char *s_temp_mix = getStr(site->cfg, (void *) key);

  key = "s_temp_evapor1";
  char *s_temp_evapor1 = getStr(site->cfg, (void *) key);

  key = "s_temp_evapor2";
  char *s_temp_evapor2 = getStr(site->cfg, (void *) key);

  // double a = get_data(conn, "/mnt/1wire/21.F2FBE3467CC2/temperature");
  // s_temp_in = "28.4AEC29CDBAAB"; // для тестов

  // для чтения на Raspberry Pi, через сервер "/bus.0/21.F2FBE3467CC2/temperature"

  char buf[100];
  char buf1[100];
  char buf2[100];
  char buf3[100];
  char buf4[100];
  char str1[100];
  
  int ret;
  struct stat st;
  if ((ret = stat(s_temp_out,&st)) != 0) {
    site->temp_out = get_data(conn, concat(mnt, s_temp_out), 100);
  } else {
    FILE *f = fopen(s_temp_out,"r");
    fgets(buf,sizeof(buf),f);
    site->temp_out = atof(buf);
    fclose(f);

  }
    printf("temp_out = %2.2f ret = %d\n",site->temp_out, ret);
  
  sprintf(str1,"%s",s_temp_in);  
  if ((ret = stat(str1,&st)) != 0) {
    site->temp_in = get_data(conn, concat(mnt, s_temp_in), 100);
  } else {
    FILE *f1 = fopen(s_temp_in,"r");
    fgets(buf,sizeof(buf1),f1);
    site->temp_in = atof(buf1);
    fclose(f1);
  }
  printf("temp_in = %2.2f ret = %d\n",site->temp_in, ret);  
  
  if (stat(s_temp_mix,&st) != 0) {
    site->temp_mix = get_data(conn, concat(mnt, s_temp_mix), 100);
  } else {
    FILE *f2 = fopen(s_temp_in,"r");
    fgets(buf,sizeof(buf2),f2);
    site->temp_mix = atof(buf2);
    fclose(f2);
  }
  
  printf("temp_mix = %2.2f\n",site->temp_mix);  
  
  
  if (stat(s_temp_evapor1,&st) != 0) {
    site->temp_evapor1 = get_data(conn, concat(mnt, s_temp_evapor1), 100);
  } else {
    FILE *f3 = fopen(s_temp_evapor1,"r");
    fgets(buf,sizeof(buf3),f3);
    site->temp_evapor1 = atof(buf3);
    fclose(f3);
  }
  
  printf("temp_evapor1 = %2.2f\n",site->temp_evapor1);  
  
  
  if (stat(s_temp_evapor2,&st) != 0) {
    site->temp_evapor2 = get_data(conn, concat(mnt, s_temp_evapor2), 100);
  } else {
    FILE *f4 = fopen(s_temp_evapor2,"r");
    fgets(buf,sizeof(buf4),f4);
    site->temp_evapor2 = atof(buf4);
    fclose(f4);
  }
  
  printf("temp_evapor2 = %2.2f\n",site->temp_evapor2);  


  return 0;

}
