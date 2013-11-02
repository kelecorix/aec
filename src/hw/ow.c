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
#include "../log/file_logger.h"

/*
 *
 *
 *
 */
OWNET_HANDLE create_server_conn(char *addr) {

  OWNET_HANDLE owh;

  owh = OWNET_init(addr);
  if (owh < 0) {
    log_2("OWNET_init(%s) failed.\n", addr);
  }
  return owh;
}

/*
 *
 *
 *
 */
int close_server_conn(OWNET_HANDLE conn) {

  OWNET_close(conn);
  return 1;
}

/*
 *
 *
 *
 */
float get_data(OWNET_HANDLE conn, char* mnt, char* filename, int lim) {

  // посмотрим или существуют файлы для эмуляции

  char* cwd = get_current_dir_name();
  char* dirpath = concat(cwd, "/");
  dirpath = concat(dirpath, gcfg->edir);
  char* fullpath = concat(dirpath, filename);
  if (access(fullpath, F_OK) != -1) {
    // Файл существует
    char buf[100];
    FILE *fp = fopen(fullpath, "r");
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

      if (strcmp(data, " ")==0) {
        // пытаемся считать еще несколько раз
        int i;
        for (i = 0; i < lim; i++) {
          OWNET_read(conn, full_path, &data);
          if (strcmp(data," ")!=0)
            break;
        }

        if (strcmp(data, " ")==0) {
          // все еще пусто после повторных опросов
          return -100.f; // вернем ошибку
        }
      }

      // уберем лидирующие пробелы
      char *datat = ltrim(data);
      float d = atof(datat);

      // Проверим или данные входят в рабочий диапазон
      // -55 .. +125
      if (d > -55.f && d < 125.f)
        return d;
      else
        return -100.f;

    } else
      return -100.f;
  }
}

/*
 *
 *
 *
 */
int list_sensors(char *tokens[]) {

  int i, ret;
  char **dirs[3];
  printf("точка монтирования %s\n", gcfg->mpoint);
  //OWNET_dirlist(gcfg->conn, NULL, dirs);
  ret = OWNET_dirlist(gcfg->conn, gcfg->mpoint, dirs);
  //printf("возвр %d\n", ret);
  printf("перед циклом %s \n\n", dirs[0]);
  splitString(dirs[0], tokens, ',');

  for (i=0;i<12;i++){
    if(strstr(tokens[i], "28") != NULL) {
      printf("sens %d: %s\n", i, tokens[i]);
    }
  }
  return 0;
}



