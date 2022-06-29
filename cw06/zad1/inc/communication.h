#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "config.h"

typedef enum {
	ECHO = 1,
	INIT,
	LIST,
	FRIENDS,
	ADD,
	DEL,
	TO_ALL,
	TO_FRIENDS,
	TO_ONE,
	STOP,
	UNRECOGNIZED
} Communicate;

typedef struct {
	long mtype;
	int clientid;
	char mtext[MAX_MSG_LEN];
} Request;

typedef struct {
	long mtype;
	char mtext[MAX_MSG_LEN];
} Response;


void init_communication();

const char *get_qpath();
int delete_queue(int id);
int get_server_queue(int create);
int get_random_projid();
int create_queue();

Communicate parse_communicate(const char *communicate_str);

// private
int generate_queue_id();


#endif
