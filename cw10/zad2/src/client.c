#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../inc/utils.h"
#include "../inc/config.h"
#include "../inc/connection.h"


int check_args(int argc, char **argv);
ConnectionProps parse_args(char **argv);

void handle_signal(int _);
int register_sighandlers();
void cleaning_ladies();

int sd;
ConnectionProps connection;
int count_job(const char *filepath);
int client_idx = -1;

		
int main(int argc, char **argv) {
	if (check_args(argc, argv) == -1) {
		exit(BAD_ARGS);
	}

    sigset_t set;
	sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    int retcode = sigprocmask(SIG_BLOCK, &set, NULL);
    if (retcode == -1) perror("sigprocmask");

	ssize_t res; 

	connection = parse_args(argv);
	sd = socket(connection.domain, CON_TYPE, 0);
	if (sd == -1) {
		perror("Error socket\n");
		exit(BAD_OP);
	}

	struct sockaddr* addr = get_sockaddr(connection);
	if (addr == NULL) {
		perror("Error getting address. Wtf\n");
		exit(BAD_OP);
	}

	if (connect(sd, addr, sizeof(*addr)) == -1) {
		perror("Error connecting\n");
		exit(BAD_OP);
	}

	if (connection.domain == AF_UNIX) {
		free_unix_addr(addr);
	} else if (connection.domain == AF_INET) {
		free_local_addr(addr);
	}

	if(register_sighandlers() == -1) {
		perror("Cannot register signal handlers\n");
		exit(BAD_OP);
	}
	
	// register client
	Message* login_message = create_message(strlen(connection.name) + 1);

	login_message->type = LOGIN;
	strcpy(login_message->content.char_val, connection.name);

	res = send(sd, login_message, sizeof(Message) + login_message->length, 0);
	if (res == -1) {
		perror("Error sending register\n");
		exit(BAD_OP);
	}
	free(login_message);

	login_message = create_message(sizeof(LoginStatus) + 1);
	res = recv(sd, login_message, sizeof(login_message) + sizeof(LoginStatus), 0); 
	if (res == -1) { 
		perror("Error receiving register response\n");
		exit(BAD_OP);
	}

	login_message->content.char_val = (char *)login_message + sizeof(Message);

	if (login_message->content.int_val[0] == FAILED) {
		perror("Register rejected\n");
		exit(BAD_OP);
	}

	client_idx = login_message->id;

	free(login_message);

	printf("I was registered!\n");
	
	Message* peek_message;
	Message* message;
	while(1) {
		peek_message = create_message(0);
	    res = recv(sd, peek_message, sizeof(*peek_message), MSG_PEEK); 
	    if (res == -1) { 
	    	perror("Error receiving communicate\n");
			continue;
	    }

		message = create_message(peek_message->length + 1);
	    res = recv(sd, message, sizeof(*message) + peek_message->length, 0); 
		message->content.char_val = (char *)message + sizeof(Message);
	    if (res == -1) { 
	    	perror("Error receiving communicate\n");
			continue;
	    }

		
		switch(message->type) {
			case JOB: {
				printf("Got job\n");

				int count = count_job(message->content.char_val);
				printf("Count is: %d\n", count);

				char *res_str = (char *)alloc(
						strlen(connection.name) + strlen(": ") + 16);
				sprintf(res_str, "%s: %d", connection.name, count);

				Message *resp = create_message(strlen(res_str) + 1);
				resp->type = JOB;
				resp->id = client_idx;
				strcpy(resp->content.char_val, res_str);

				res = send(sd, resp,
						sizeof(Message) + resp->length, 0);
				if (res == -1) {
					perror("Error sending JOB response\n");
				}

				free(res_str);
				free(resp);
				break;
		  	}
			case PING: {
				printf("Responding to ping\n");

				Message *resp = create_message(0);
				resp->type = PING;
				resp->id = client_idx;	

				res = send(sd, resp,
						sizeof(Message) + resp->length, 0);
				if (res == -1) {
					perror("Error sending PING response\n");
				}

				free(resp);
				break;
			}
			default:
				printf("Ya default? %d\n", message->type);
				break;
		}

		free(message);
		free(peek_message);
	}

	return 0;
}


/**
 * Arguments:
 * name
 * conection type - UNIX or LOCAL 
 * unix socket path or ip
 * [optional] port (if type is AF_INET)
 **/
int check_args(int argc, char **argv) {
	if (argc != 4 && argc != 5) {
		perror("Wrong arguments number. Expected 3 or 4\n");
		return -1;
	}

	if (strcmp(argv[2], "UNIX") == 0 && strcmp(argv[2], "LOCAL") == 0) {
		perror("Second argument must be UNIX or LOCAL\n");
		return -1;
	}

	if (strcmp(argv[2], "LOCAL") == 0 &&
			(argc != 5 || !is_num(argv[4]))) {
		perror("If you specify LOCAL, port must be 4th argument\n");
		return -1;
	}

	return 0;
}

ConnectionProps parse_args(char **argv) {
	ConnectionProps connection;

	connection.name = (char *)alloc(strlen(argv[1]) + 1);
	strcpy(connection.name, argv[1]);

	connection.domain = parse_connection(argv[2]);

	if (connection.domain == AF_INET) {
		strcpy(connection.ip, argv[3]);

		connection.port = (int)strtol(argv[4], NULL, 10); 
		connection.sockpath = NULL;
	} else {
		connection.sockpath = (char *)alloc(strlen(argv[3]) + 1);
		strcpy(connection.sockpath, argv[3]);
		connection.port = -1;
	}

	return connection;
}

int register_sighandlers() {
	if (atexit(cleaning_ladies) != 0) {
		perror("Error calling cleaning ladies!\n");
		return -1;
	}

	struct sigaction act;
	act.sa_handler = handle_signal;
	sigemptyset(&act.sa_mask);

	if (sigaction(SIGINT, &act, NULL) == -1) {
		perror("Sigaction error\n");
		return -1;
	}

	return 0;
}

void handle_signal(int a) {
	a = a + 1; // for ycm not to complain

	Message* logout = create_message(0);
	logout->type = LOGOUT;
	logout->id = client_idx;

	ssize_t res = send(sd, logout, sizeof(Message) + logout->length, 0);
	free(logout);
	if (res == -1) {
		perror("Error sending logout\n");
		exit(-1);
	}

	exit(0);
}

void cleaning_ladies() {
	free(connection.name);
	if (connection.sockpath != NULL) {
		free(connection.sockpath);
	}
}

int count_job(const char *buf) {
	int res = 0;
	int fsize = strlen(buf);
	for (int i = 0; i < fsize; ++i) {
		if (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\t') {
			++res;
		}
	}

	return res;
}
