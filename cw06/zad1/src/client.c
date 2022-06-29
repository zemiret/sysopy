#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "../inc/config.h"
#include "../inc/communication.h"
#include "../inc/utils.h"


void register_stop_handler();
void on_sigint(int _);
void on_stop();

int init(int client_qid, int server_qid);

void echo(int server_qid, int clientid, int client_qid, const char *text);
void list(int server_qid, int clientid);
void friends(int server_qid, int clientid, const char *text);
void add(int server_qid, int clientid, const char *text);
void del(int server_qid, int clientid, const char *text);
void to_all(int server_qid, int clientid, const char *text);
void to_friends(int server_qid, int clientid, const char *text);
void to_one(int server_qid, int clientid, const char *text);

void execute_from_stdin();
void execute_from_file(const char *filename);
void execute_line(char *line);

void handle_message(Response msg);

void send_simple(
		int server_qid,
		Communicate ctype,
		const char *ctype_str,
		int clientid,
		const char *text);


int client_qid; 
int server_qid; 
int clientid;

int main(int argc, char **argv) {
	init_communication();
	atexit(on_stop);

	server_qid = get_server_queue(0);
	if (server_qid == -1) {
		perror("Cannot get server queue\n");
		exit(-1);
	}

	client_qid = create_queue();
	if (client_qid == -1) {
		perror("Cannot create queue\n");
		exit(-1);
	}

	clientid = init(client_qid, server_qid);
	register_stop_handler();

	printf("Server qid: %d\n", server_qid);
	printf("Client qid: %d\n", client_qid);
	printf("clientid: %d\n", clientid);


	if (argc == 1) {
		execute_from_stdin();
	} else {
		execute_from_file(argv[1]);
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
	Request stop_request;
	stop_request.clientid = clientid;
	stop_request.mtype = STOP;
	strcpy(stop_request.mtext, "");

	if (msgsnd(server_qid, &stop_request,
				sizeof(stop_request) - sizeof(stop_request.mtext), 0) == -1) {
		perror("Error sending stop request\n");
	}

	exit(0);
}

void on_stop() {
	printf("Stopping client queue: %d\n", client_qid);
	delete_queue(client_qid);
}

int init(int client_qid, int server_qid) {
	Request initreq;
	initreq.mtype = INIT;
	sprintf(initreq.mtext, "%d", client_qid);

	if (msgsnd(server_qid, &initreq,
				sizeof(initreq) - sizeof(initreq.mtype), 0) == -1) {
		perror("Error sending init request\n");
		exit(-1);
	};

	Response initresp;
	if (msgrcv(
				client_qid,
				&initresp,
				sizeof(initreq) - sizeof(initreq.mtype),
				INIT, 0) == -1) {
		perror("Error receiving init response\n");
		exit(-1);
	}; 

	int preverrno = errno;
	int clientid = (int)strtol(initresp.mtext, NULL, 10);
	if (clientid == 0 && preverrno != errno) {
		perror("Cannot read clientid from server response\n");
		exit(-1);
	}

	return clientid;
}

void echo(int server_qid, int clientid, int client_qid, const char *text) {
	send_simple(server_qid, ECHO, "echo", clientid, text);

	Response echoresp;

    if (msgrcv(client_qid,
				&echoresp,
				sizeof(echoresp) - sizeof(echoresp.mtype), ECHO, 0) == -1) {
    	perror("Error receiving echo response\n");
    	exit(-1);
    }; 

	printf("%s\n", echoresp.mtext);
	fflush(stdout);
}


void list(int server_qid, int clientid) {
	send_simple(server_qid, LIST, "list", clientid, "");
}

void friends(int server_qid, int clientid, const char *text) {
	send_simple(server_qid, FRIENDS, "friends", clientid, text);
}

void add(int server_qid, int clientid, const char *text) {
	if (is_empty(text)) {
		perror("ADD error: Cannot send empty add request\n");
	}
	send_simple(server_qid, ADD, "add", clientid, text);
}

void del(int server_qid, int clientid, const char *text) {
	if (is_empty(text)) {
		perror("DEL error: Cannot send empty add request\n");
	}
	send_simple(server_qid, DEL, "add", clientid, text);
}

void to_all(int server_qid, int clientid, const char *text) {
	send_simple(server_qid, TO_ALL, "2all", clientid, text);
}

void to_friends(int server_qid, int clientid, const char *text) {
	send_simple(server_qid, TO_FRIENDS, "2friends", clientid, text);
}

void to_one(int server_qid, int clientid, const char *text) {
	send_simple(server_qid, TO_ONE, "2one", clientid, text);
}

void send_simple(
		int server_qid,
		Communicate ctype,
		const char *ctype_str,
		int clientid,
		const char *text) {
	
	Request req;
	req.mtype = ctype;
	req.clientid = clientid;
	strcpy(req.mtext, text);

	if(msgsnd(server_qid,
				&req,
				sizeof(req) - sizeof(req.mtype), 0) == -1) {
		fprintf(stderr, "Error sending %s request to: %d\n", ctype_str, server_qid);
	}
}

void execute_from_stdin() {
	size_t n;
	char *line;

	if(fork() == 0) {
		while(1) {
		    Response server_msg;
		    int res = msgrcv(
		    		client_qid,
		    		&server_msg,
		    		sizeof(server_msg) - sizeof(server_msg.mtype),	
		    		0,
		    		IPC_NOWAIT); 

		    if (res != -1) {
		    	handle_message(server_msg);
		    }
		}
	} else {
    	while(1) {
    		n = 0;
    		line = NULL;

    		if (getline(&line, &n, stdin) == -1) {
    			perror("Cannot read line\n");
    		} else if(!is_empty(line)) {
    			execute_line(line);
    		}
    
    		free(line);
    	}
	}
}

void execute_line(char *line) {
	char copy[MAX_MSG_LEN];
	strcpy(copy, line);
	
	char *type = strtok(copy, " \t\n");
	char text_no_type[MAX_MSG_LEN];
	strncpy(text_no_type, line + strlen(type), strlen(line) - strlen(type) + 1);

	char *text = NULL;
	trim_string(&text, text_no_type);

	switch(parse_communicate(type)) {
		case ECHO:
			echo(server_qid, clientid, client_qid, text); 
			break;
		case LIST:
			list(server_qid, clientid);
			break;
		case FRIENDS:
			friends(server_qid, clientid, text);
			break;
		case ADD:
			add(server_qid, clientid, text);
			break;
		case DEL:
			del(server_qid, clientid, text);
			break;
		case TO_ALL:
			to_all(server_qid, clientid, text);
			break;
		case TO_FRIENDS:
			to_friends(server_qid, clientid, text);
			break;
		case TO_ONE:
			to_one(server_qid, clientid, text);
			break;
		default:
			perror("Unknown communicate\n");
			break;
	}

	free(text);
}

void handle_message(Response resp) {
	switch(resp.mtype) {
		case STOP:
			kill(getppid(), SIGINT);
			exit(0);
			break;
		case TO_ONE:
		case TO_ALL:
			printf("%s\n", resp.mtext);
			break;
	}
}

void execute_from_file(const char *filename) {
	if(fork() == 0) {
		while(1) {
		    Response server_msg;
		    int res = msgrcv(
		    		client_qid,
		    		&server_msg,
		    		sizeof(server_msg) - sizeof(server_msg.mtype),	
		    		0,
		    		IPC_NOWAIT); 

		    if (res != -1) {
		    	handle_message(server_msg);
		    }
		}
	} else {
		FILE *cmdfile = fopen(filename, "r");
		size_t n = 0;
		char *line = NULL;

    	while(getline(&line, &n, cmdfile) != -1) {
			if (!is_empty(line)) {
    			execute_line(line);
			}

    		free(line);
			n = 0;
			line = NULL;

			sleep(1);
    	}

		fclose(cmdfile);
	}
}
