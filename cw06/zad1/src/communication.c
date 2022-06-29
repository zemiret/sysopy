#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../inc/communication.h"
#include "../inc/config.h"


void init_communication() {
	srand(time(NULL));
}

const char *get_qpath() {
	return getenv("HOME");
}

int get_server_queue(int create) {
	key_t id = ftok(get_qpath(), SERVER_QUEUE_ID);
	if (id == -1) {
		return -1;
	}

	if (create) {
		return msgget(id, IPC_CREAT | 0666);
	} else {
		return msgget(id, 0);
	}
}

int get_random_projid() {
	// Potential threat of having 2 same ids generated
	int res = rand();
	while (res == SERVER_QUEUE_ID) {
		res = rand();
	}
	return res;
}

int create_queue() {
	key_t id = ftok(get_qpath(), get_random_projid());
	if (id == -1) {
		return -1;
	}

	return msgget(id, IPC_CREAT | 0666);
}

int delete_queue(int qid) {
	return msgctl(qid, IPC_RMID, NULL);
}	

Communicate parse_communicate(const char *communicate_str) {
	Communicate res;

	if (strcmp(communicate_str, "ECHO") == 0) {
		res = ECHO; } else if (strcmp(communicate_str, "INIT") == 0) {
		res = INIT;
	} else if (strcmp(communicate_str, "LIST") == 0) {
		res = LIST;
	} else if (strcmp(communicate_str, "FRIENDS") == 0) {
		res = FRIENDS;
	} else if (strcmp(communicate_str, "ADD") == 0) {
		res = ADD;
	} else if (strcmp(communicate_str, "DEL") == 0) {
		res = DEL;
	} else if (strcmp(communicate_str, "2ALL") == 0) {
		res = TO_ALL;
	} else if (strcmp(communicate_str, "2FRIENDS") == 0) {
		res = TO_FRIENDS;
	} else if (strcmp(communicate_str, "2ONE") == 0) {
		res = TO_ONE;
	} else if (strcmp(communicate_str, "STOP") == 0) {
		res = STOP;
	} else {
		res = UNRECOGNIZED;
	}

	return res;
}

