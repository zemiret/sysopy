#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <mqueue.h>
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

typedef struct {
	int qid;
	char *qpath;
} Qinit;

void init_communication();

const char *get_qpath();
int delete_queue(mqd_t id);
mqd_t get_server_queue(int create);
char * get_random_projid();
Qinit create_queue();
mqd_t open_queue(char *qpath);

// converting functions
Request to_request(char *req_str);
char *from_request(Request req);
Response to_response(char *resp_str);
char *from_response(Response resp);

Communicate parse_communicate(const char *communicate_str);

// private
int generate_queue_id();


#endif
