#ifndef OW_H_
#define OW_H_

#include <string.h>
#include <ownetapi.h>
#include <stddef.h>

OWNET_HANDLE create_server_conn(char *addr);
int close_server_conn(OWNET_HANDLE srv);
float get_data(OWNET_HANDLE conn, char *mnt, char *filename, int lim);
int list_sensors(char *tokens[]);

#endif /* OW_H_ */
