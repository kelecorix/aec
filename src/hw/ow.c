#include <string.h>
#include <ownetapi.h>
#include <stddef.h>
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
    }

    // Считываение прошло нормально

    if (data == "") {
      // пытаемся считать еще несколько раз
      int i;
      for (i=0; i<lim; i++){
        OWNET_read(conn, full_path, &data);
        if (data != "")
          break;
      }

      if (data == ""){
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
  s_temp_in = "28.4AEC29CDBAAB"; // для тестов

  // для чтения на Raspberry Pi, через сервер "/bus.0/21.F2FBE3467CC2/temperature"

  site->temp_out = get_data(conn, concat(mnt, s_temp_out), 100);
  site->temp_in = get_data(conn, concat(mnt, s_temp_in), 100);
  site->temp_mix = get_data(conn, concat(mnt, s_temp_mix), 100);
  site->temp_evapor1 = get_data(conn, concat(mnt, s_temp_evapor1), 100);
  site->temp_evapor2 = get_data(conn, concat(mnt, s_temp_evapor2), 100);

  return 0;

}
