#ifndef BOX_UTIL_H
#define BOX_UTIL_H

#include <inttypes.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
class box_event;
class box_event_sock;

typedef void(*box_read_callback)(box_event_sock*);
typedef void(*box_write_callback)(box_event_sock*);

void box_set_nonblock(int fd);
int box_create_server(uint16_t port, const char* ip = "0.0.0.0");

#define box_err printf
#define box_debug printf

#endif // BOX_UTIL_H
