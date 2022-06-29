#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include "../inc/utils.h"
#include "../inc/communication.h"
#include "../inc/config.h"


void init_communication() {
	srand(time(NULL));
}

mqd_t get_server_queue(int create) {
	struct mq_attr qattrs;
	qattrs.mq_maxmsg = 10;
	qattrs.mq_msgsize = REQUEST_LEN;

	if (create) {
		return mq_open(SERVER_QUEUE_ID, O_RDWR | O_CREAT, 0666, &qattrs);
	} else {
		return mq_open(SERVER_QUEUE_ID, O_RDWR);
	}
}

char* get_random_projid() {
	// Potential threat of having 2 same ids generated
	// REMEMBER TO FREE!
	char *res = (char *)alloc(strlen(BASE_QUEUE_ID) + 11);
	sprintf(res, "%s%d", BASE_QUEUE_ID, rand());
	return res;
}


Qinit create_queue() {
	char *qpath = get_random_projid();

	struct mq_attr qattrs;
	qattrs.mq_maxmsg = 10;
	qattrs.mq_msgsize = REQUEST_LEN;

	mqd_t qid = mq_open(qpath, O_RDWR | O_CREAT, 0666, &qattrs);

	Qinit res;
	res.qid = qid;
	res.qpath = qpath;
	return res;
}

mqd_t open_queue(char *qpath) {
	struct mq_attr qattrs;
	qattrs.mq_maxmsg = 10;
	qattrs.mq_msgsize = REQUEST_LEN;

	mqd_t qid = mq_open(qpath, O_RDWR | O_CREAT, 0666, &qattrs);

	return qid;
}

int delete_queue(mqd_t qid) {
	return mq_close(qid);
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

Request to_request(char *req_str) {
	char *copy = (char *)alloc(strlen(req_str) + 1);
	strcpy(copy, req_str);

	char* type_str = strtok(copy, " \t\n");
	char* clientid_str = strtok(NULL, " \t\n");

	Request res;

	int preverrno = errno;
	res.mtype = strtol(type_str, NULL, 10);
	if (res.mtype == 0 && preverrno != errno) {
		fprintf(stderr, "Error converting %s to request", req_str);
	}

	preverrno = errno;
	res.clientid = (int)strtol(clientid_str, NULL, 10);
	if (res.mtype == 0 && preverrno != errno) {
		fprintf(stderr, "Error converting %s to request", req_str);
	}

	strncpy(res.mtext,
			req_str + strlen(type_str) + strlen(clientid_str) + 2,
			MAX_MSG_LEN);

	free(type_str);
	free(req_str);
	return res;
}

char *from_request(Request req) {
	// Remember about freeing this ptr!
	char *res = (char *)alloc(REQUEST_LEN); 
	sprintf(res, "%ld %d %s", req.mtype, req.clientid, req.mtext);
	return res;
}

Response to_response(char *resp_str) {
	char *copy = (char *)alloc(strlen(resp_str) + 1);
	strcpy(copy, resp_str);

	char* type_str = strtok(copy, " \t\n");

	Response res;

	int preverrno = errno;
	res.mtype = (int)strtol(type_str, NULL, 10);
	if (res.mtype == 0 && preverrno != errno) {
		fprintf(stderr, "Error converting %s to request", resp_str);
	}

	strncpy(res.mtext,
			resp_str + strlen(type_str) + 1,
			MAX_MSG_LEN);

	free(type_str);
	free(resp_str);
	return res;
}

char *from_response(Response resp) {
	char *res = (char *)alloc(RESPONSE_LEN); 
	sprintf(res, "%ld %s", resp.mtype, resp.mtext);
	return res;
}
