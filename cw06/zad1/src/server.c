#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include "../inc/communication.h"
#include "../inc/config.h"
#include "../inc/utils.h"

void handle_init(
		int client_queues[MAX_CLIENTS],
		int *created_clients_count,
		Request msg);

void handle_echo(
		int client_queues[MAX_CLIENTS],
		Request msg);

void handle_stop(
		int client_queues[MAX_CLIENTS],
		int *created_clients_count,
		Request msg);

void handle_friends(
		int friends[MAX_CLIENTS],
		Request msg);

void handle_add(
		int friends[MAX_CLIENTS],
		Request msg);

void handle_del(
		int friends[MAX_CLIENTS],
		Request msg);

void handle_to_all(
		int client_queues[MAX_CLIENTS],
		Request msg);

void handle_to_friends(
		int client_queues[MAX_CLIENTS],
		int friends[MAX_CLIENTS],
		Request msg);

void handle_to_one(
		int client_queues[MAX_CLIENTS],
		Request msg);

void handle_list(int client_queues[MAX_CLIENTS]);

void register_stop_handler(); 
void on_sigint(int sig);
void on_stop();

int server_qid;
int created_clients_count;
int client_queues[MAX_CLIENTS]; // position is client id at the same time

int main() {
	init_communication();
	atexit(on_stop);

	int friends[MAX_CLIENTS];
	created_clients_count = 0;

	server_qid = get_server_queue(1); 
	if (server_qid == -1) {
		perror("Unable to create server queue\n");
		exit(-1);
	}

	printf("Server qid: %d\n", server_qid);

	for (int i = 0; i < MAX_CLIENTS; ++i) {
		client_queues[i] = -1;
		friends[i] = -1;
	}

	register_stop_handler();

	Request request;
	while(1) {
		int res = msgrcv(
				server_qid,
				&request,
				sizeof(request) - sizeof(request.mtype),	
				PRIO_MSGTYPE,
				MSG_NOERROR); 
		printf("Received msg type: %ld\n", request.mtype);

		if (res == -1) {
			continue;
		}

		switch (request.mtype) {
			case INIT:
				handle_init(client_queues, &created_clients_count, request);
				break;
			case ECHO:
				handle_echo(client_queues, request);
				break;
			case LIST:
				handle_list(client_queues);
				break;
			case FRIENDS:
				handle_friends(friends, request);
				break;
			case ADD:
				handle_add(friends, request);
				break;
			case DEL:
				handle_del(friends, request);
				break;
			case TO_FRIENDS:
				handle_to_friends(client_queues, friends, request);
				break;
			case TO_ALL:
				handle_to_all(client_queues, request);
				break;
			case TO_ONE:
				handle_to_one(client_queues, request);
				break;
			case STOP:
				handle_stop(client_queues, &created_clients_count, request);
				break;
			default:
				break;
		}
	}

	return 0;
}

void register_stop_handler() {
	struct sigaction act;
	act.sa_handler = on_sigint;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	sigaction(SIGINT, &act, NULL);
}

void on_sigint(int _) {
	Response resp;
	resp.mtype = STOP;

	printf("Stopping, clients count: %d\n", created_clients_count);

	// send stop to all clients
	for (int i = 0; i < MAX_CLIENTS; ++i) {
		if (client_queues[i] != -1) {
			if(msgsnd(client_queues[i],
						&resp,
						sizeof(resp) - sizeof(resp.mtype), 0) == -1) {
				fprintf(stderr, "Cannot send stop to: %d\n", client_queues[i]);
			}
		}
	}

	// receive stop from all clients
	int received_stops_count = 0;
	Request req;
	while (received_stops_count < created_clients_count) {
		if(msgrcv(server_qid,
					&req,
					sizeof(req) - sizeof(req.mtype),
					STOP, 0) == -1) {
			perror("Error receiving stop response. Lul\n");
		}

		++received_stops_count;
	}

	exit(0);
}

void on_stop() {
	printf("Stopping server queue: %d\n", server_qid);
	delete_queue(server_qid);
}

void handle_init(
		int client_queues[MAX_CLIENTS],
		int *created_clients_count,
		Request msg) {

	if (*created_clients_count == MAX_CLIENTS) {
		fprintf(stderr,"Cannot create another client. Max limit is: %d\n", MAX_CLIENTS); 
		return;
	}

	// get client queue id
	int preverrno = errno;
	int qid = (int)strtol(msg.mtext, NULL, 10); 

	if (qid == 0 && errno != preverrno) {
		perror("Cannot extract client id from INIT message\n");
		return;
	}

	// register client
	int client_id = 0;
	while (client_id < MAX_CLIENTS) {
		if (client_queues[client_id] == -1) {
			break;
		}
		++client_id;
	}

	client_queues[client_id] = qid; 

	++(*created_clients_count);

	// respond
	printf("INIT %d\n", client_id);
	
	Response response;
	response.mtype = INIT;
	sprintf(response.mtext, "%d", client_id);
	if(msgsnd(qid, &response, sizeof(response) - sizeof(response.mtype), 0) == -1) {
		fprintf(stderr, "Error sending response to client: %d\n", qid);	
		return;
	};
}

void handle_echo(int client_queues[MAX_CLIENTS], Request msg) {
	printf("ECHO clientid: %d, client_qid: %d\n",
			msg.clientid,
			client_queues[msg.clientid]);

	char *date = get_date_str();

	char client_msg[MAX_MSG_LEN];
	strcpy(client_msg, date);
	strcat(client_msg, " ");
	strcat(client_msg, msg.mtext);

	Response response;
	response.mtype = ECHO;
	strcpy(response.mtext, client_msg);

	if (msgsnd(
			client_queues[msg.clientid],
			&response,
			sizeof(response) - sizeof(response.mtype), 0) == -1) {
		fprintf(stderr, "Error: Response ECHO clientid: %d, client_qid: %d\n",
			msg.clientid,
			client_queues[msg.clientid]);
	}

	free(date);
}

void handle_stop(
		int client_queues[MAX_CLIENTS],
		int *created_clients_count,
		Request msg) {
	printf("STOP clientid: %d, client_qid: %d\n",
			msg.clientid, client_queues[msg.clientid]);

	client_queues[msg.clientid] = -1;
	--(*created_clients_count);
}

void handle_list(int client_queues[MAX_CLIENTS]) {
	printf("LIST\n");
	for (int i = 0; i < MAX_CLIENTS; ++i) {
		if (client_queues[i] != -1) {
			printf("clientid: %d, client_qid: %d\n", i, client_queues[i]);
		}
	}
}

void handle_friends(
		int friends[MAX_CLIENTS],
		Request msg) {
	printf("FRIENDS: %s\n", msg.mtext);

	for (int i = 0; i < MAX_CLIENTS; ++i) {
		friends[i] = -1;
	}

	if (is_empty(msg.mtext)) {
		return;
	}

	char *token = strtok(msg.mtext, " \t\n");
	int i = 0;
	while(token != NULL && i < MAX_CLIENTS) {
		int preverrno = errno;
		int friend_id = (int)strtol(token, NULL, 10);		

		if (friend_id == 0 && preverrno != errno) {
			perror("FRIENDS errror: Cannot parse friend id\n");
		} else if(friend_id >= 0 && friend_id < MAX_CLIENTS) {
			friends[i] = friend_id;
			++i;
		}
		token = strtok(NULL, " \t\n");
	}
}

void handle_add(int friends[MAX_CLIENTS], Request msg) {
	int i = 0;
	while (i < MAX_CLIENTS && friends[i] != -1) {
		++i;
	}
	if (i == MAX_CLIENTS) {
		perror("ADD Error: Max friends capacity reached");
	} else {
		int preverrno = errno;
		int friend_id = (int)strtol(msg.mtext, NULL, 10);		

		if (friend_id == 0 && preverrno != errno) {
			perror("ADD Error: Cannot parse friend id\n");
		} else {
			printf("ADD id: %d\n", friend_id);
			friends[i] = friend_id;
		}
	}
}

void handle_del(int friends[MAX_CLIENTS], Request msg) {
	int preverrno = errno;
	int friend_id = (int)strtol(msg.mtext, NULL, 10);		

	if (friend_id == 0 && preverrno != errno) {
		perror("DEL Error: Cannot parse friend id\n");
		return;
	} 

	int i = 0;
	while (i < MAX_CLIENTS && friends[i] != friend_id) {
		++i;
	}

	if (i == MAX_CLIENTS) {
		fprintf(stderr, "DEL error: Friend %d not found\n", friend_id);
	} else {
		friends[i] = -1;
	}
}


void handle_to_all(
		int client_queues[MAX_CLIENTS],
		Request msg) {
	printf("2ALL %s\n", msg.mtext);

	char *text = msg.mtext;
	char from[10];
	sprintf(from, "%d", msg.clientid);
	char *date = get_date_str();

	Response resp;
	resp.mtype = TO_ALL;

	strcpy(resp.mtext, from);
	strcat(resp.mtext, " ");
	strcat(resp.mtext, date);
	strcat(resp.mtext, " ");
	strcat(resp.mtext, text);

	for (int i = 0; i < MAX_CLIENTS; ++i) {
		if (client_queues[i] != -1) {
    	    if (msgsnd(
    	    		client_queues[i],
    	    		&resp,
    	    		sizeof(resp) - sizeof(resp.mtype), 0) == -1) {
    	    	fprintf(stderr, "2ALL error: client_qid: %d\n",
    	    		client_queues[i]);
    	    }
		}
	}

	free(date);
}

void handle_to_friends(
		int client_queues[MAX_CLIENTS],
		int friends[MAX_CLIENTS],
		Request msg) {

	printf("2FRIENDS %s\n", msg.mtext);
	int friends_qids[MAX_CLIENTS];

	for (int i = 0; i < MAX_CLIENTS; ++i) {
		if (friends[i] != -1) {
			friends_qids[i] = client_queues[friends[i]];
		} else {
			friends_qids[i] = -1;
		}
	}

	handle_to_all(friends_qids, msg);
}

void handle_to_one(
		int client_queues[MAX_CLIENTS],
		Request msg) {
	printf("2ONE %s\n", msg.mtext);

	// parse adresee
	char text[MAX_MSG_LEN];
	char copy[MAX_MSG_LEN];

	strcpy(copy, msg.mtext);
	char *to = strtok(copy, " \n\t");

	strncpy(text, msg.mtext + strlen(to), strlen(msg.mtext) - strlen(to) + 1);
	
	int preverrno = errno;
	int to_id = (int)strtol(to, NULL, 10);

	if (to_id == 0 && errno != preverrno) {
		perror("2ONE error: Cannot parse adresee id\n");
		return;
	}

	if (to_id < 0 || to_id >= MAX_CLIENTS) {
		perror("2ONE error: Invalid adresee\n");
		return;
	}

	char from[10];
	sprintf(from, "%d", msg.clientid);
	char *date = get_date_str();

	Response resp;
	resp.mtype = TO_ONE;

	strcpy(resp.mtext, from);
	strcat(resp.mtext, " ");
	strcat(resp.mtext, date);
	strcat(resp.mtext, " ");
	strcat(resp.mtext, text);


	if (client_queues[to_id] != -1) {
        if (msgsnd(
        		client_queues[to_id],
        		&resp,
        		sizeof(resp) - sizeof(resp.mtype), 0) == -1) {
        	fprintf(stderr, "2ONE error: client_qid: %d\n",
        		client_queues[to_id]);
        }
	}
	
	free(date);
}

