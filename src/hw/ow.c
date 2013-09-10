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

float get_data(OWNET_HANDLE conn, char* mnt, char* filename, int lim) {

  // check if exists local file
  if (access(filename, F_OK) != -1) {
    // File exists
    // Файл существует
    char buf[100];
    FILE *fp = fopen(filename, "r");
    fgets(buf, sizeof(buf), fp);
    fclose(fp);
    return atof(buf);
  } else {

    // File doesn't exist
    // Reading exactly from sensors
    // Файл не существует
    // Производим чтение с датчиков

    char *data = NULL;
    char *temp = "/temperature";
    char *filepath = concat(mnt, filename);
    char *full_path = concat(filepath, temp);

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
        for (i = 0; i < lim; i++) {
          OWNET_read(conn, full_path, &data);
          if (data != " ")
            break;
        }

        if (data == " ") {
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
}

int list_sensors(Site* site, char *tokens[]) {

  char *dirs = NULL;
  int res = OWNET_dirlist(site->conn, site->mount_point, dirs);
  int ret = splitString(dirs, tokens, ',');

  return 0;
}

int read_sensors(Site* site) {

  OWNET_HANDLE conn = site->conn;
  char *mnt = site->mount_point;

  char *s_temp_out = getStr(site->cfg, (void *) "s_temp_outdoor");
  char *s_temp_in = getStr(site->cfg, (void *) "s_temp_indoor");
  char *s_temp_mix = getStr(site->cfg, (void *) "s_temp_mix");
  char *s_temp_evapor1 = getStr(site->cfg, (void *) "s_temp_evapor1");
  char *s_temp_evapor2 = getStr(site->cfg, (void *) "s_temp_evapor2");

  site->temp_out = get_data(conn, mnt, s_temp_out, 100);
  site->temp_in = get_data(conn, mnt, s_temp_in, 100);
  site->temp_mix = get_data(conn, mnt, s_temp_mix, 100);
  site->temp_evapor1 = get_data(conn, mnt, s_temp_evapor1, 100);
  site->temp_evapor2 = get_data(conn, mnt, s_temp_evapor2, 100);

  printf("temp_out = %2.2f\n", site->temp_out);
  printf("temp_in = %2.2f\n", site->temp_in);
  printf("temp_mix = %2.2f\n", site->temp_mix);
  printf("temp_evapor1 = %2.2f\n", site->temp_evapor1);
  printf("temp_evapor2 = %2.2f\n", site->temp_evapor2);

  return 0;

}

