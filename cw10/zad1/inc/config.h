#ifndef CONFIG_H
#define CONFIG_H

#include <sys/types.h>
#include <sys/socket.h>

#define BAD_ARGS -121
#define BAD_OP -122

#define MAX_CLIENT_COUNT 16
#define CON_TYPE SOCK_STREAM
#define BACKLOG_SIZE 128

#endif
