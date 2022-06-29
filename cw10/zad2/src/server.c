#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../inc/utils.h"
#include "../inc/config.h"
#include "../inc/connection.h"

#define MAX_EVENTS 128 

typedef struct {
	char *name;
	int job_count;
	int sd;
	unsigned long long last_ping;
} Client;


int request_id = 0;
int unix_sd;
int local_sd;
int epollfd;
char *sockname;

struct sockaddr* unix_addr = NULL;
struct sockaddr* local_addr = NULL;

Client clients[MAX_CLIENT_COUNT];

int check_args(int argc, char **argv);

void create_unix_socket(char *sockpath);
void create_local_socket(int port_number);

void handle_signal(int _);
int register_sighandlers();
void cleaning_ladies();

void* ping_routine(void *args) {
	args = NULL;
	static unsigned long long delta_dergister = 5000;

	while(1) {
	    for (int i = 0; i < MAX_CLIENT_COUNT; ++i) {
			if (clients[i].name != NULL) {
				unsigned long long cur_timestamp = get_timestamp();		

				if (clients[i].last_ping != 0 &&
						(cur_timestamp - clients[i].last_ping) > delta_dergister) {
		            clients[i].name = NULL;
		            clients[i].job_count = 0;
		            clients[i].sd = -1;
		            clients[i].last_ping = 0;

					continue;	
				}

		        Message *ping = create_message(0);

				ping->type = PING;
				ssize_t sres = send(clients[i].sd, ping,
						sizeof(Message) + ping->length, 0);
				free(ping);

				if (sres == -1) {
					fprintf(stderr, "Error sending ping to client idx: %d\n", i);
				}
			}
	    }

		sleep(1);
	}

	return NULL;
}

void* listen_routine(void *args) {
	args = NULL;
	epollfd = epoll_create1(0);
	if (epollfd == -1) {
		perror("Error creating epoll fd\n");
		exit(BAD_OP);
	}
	struct epoll_event ev, events[MAX_EVENTS];
	int nfds;
	ssize_t sres;

	if (epollfd == -1) {
		perror("Error creating epoll fd\n");
		exit(BAD_OP);
	}

	ev.events = EPOLLIN | EPOLLPRI;
	ev.data.fd = unix_sd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, unix_sd, &ev) == -1) {
		perror("Error adding unix socket to epoll\n");
		exit(BAD_OP);
	}

	ev.data.fd = local_sd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, local_sd, &ev) == -1) {
		perror("Error adding unix socket to epoll\n");
		exit(BAD_OP);
	}

	Message* peek_message;
	Message* message;

	while(1) {
		nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1); 
		if (nfds == -1) {
			perror("Error epoll_wait\n");
			continue;
		}

		for (int i = 0; i < nfds; ++i) {
			int sd = events[i].data.fd;

			if (sd == unix_sd) {
				printf("Accepting unix client\n");
				socklen_t addrlen = sizeof(*unix_addr);
				int client_sd = accept(sd, unix_addr, &addrlen);
	            ev.data.fd = client_sd;
	            if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sd, &ev) == -1) {
	            	perror("Error adding client unix socket to epoll\n");
	            	exit(BAD_OP);
	            }
			} else if (sd == local_sd) {
				printf("Accepting local client\n");
				socklen_t addrlen = sizeof(*unix_addr);
				int client_sd = accept(sd, unix_addr, &addrlen);
	            ev.data.fd = client_sd;
	            if (epoll_ctl(epollfd, EPOLL_CTL_ADD, client_sd, &ev) == -1) {
	            	perror("Error adding client local socket to epoll\n");
	            	exit(BAD_OP);
	            }
			} else {

			    peek_message = create_message(0);
	            sres = recv(sd, peek_message, sizeof(Message), MSG_PEEK); 
	            if (sres == -1) { 
	            	perror("Error receiving peek communicate\n");
					continue;
	            }

		        message = create_message(peek_message->length + 1);
	            sres = recv(sd, message, sizeof(Message) + peek_message->length, 0); 
				message->content.char_val = (char*)message + sizeof(Message);
	            if (sres == -1) { 
	            	perror("Error receiving communicate\n");
		        	continue;
	            }

			    switch(message->type) {
			    	case LOGIN: {
			    		printf("Registering: %s\n", message->content.char_val);
    		    		int cidx = 0;
    		    		while(cidx < MAX_CLIENT_COUNT && clients[cidx].name != NULL) {
    		    			++cidx;
    		    		}
    		    		if (cidx == MAX_CLIENT_COUNT) {
		        		    Message *resp = create_message(sizeof(LoginStatus) + 1);

						    resp->type = LOGIN;
						    resp->content.int_val[0] = FAILED;
						    resp->id = cidx;
						    sres = send(sd, resp,
						    		sizeof(Message) + resp->length, 0);
						    free(resp);
						    if (sres == -1) {
						    	perror("Error sending register response\n");
						    	break;
						    }

    		    			perror("Clients max reached. Cannot register.\n");
							break;
    		    		}

						clients[cidx].name = (char *)alloc(message->length + 1); 
						strcpy(clients[cidx].name, message->content.char_val);
						clients[cidx].sd = sd; clients[cidx].job_count = 0;

		        		Message *resp = create_message(sizeof(LoginStatus) + 1);

						resp->type = LOGIN;
						resp->content.int_val[0] = OK;
						resp->id = cidx;
						sres = send(sd, resp,
								sizeof(Message) + resp->length, 0);
						free(resp);
						if (sres == -1) {
							perror("Error sending register response\n");
							break;
						}
			    		printf("Registered: %s\n", message->content.char_val);
						break;
					}
			    	case PING: {
			    		clients[message->id].last_ping =
							get_timestamp();
						break;
					}
			    	case LOGOUT: {
						int cidx = message->id;

						printf("Logging out: %d: %s\n", cidx, clients[cidx].name);

                    	if (shutdown(clients[cidx].sd, SHUT_RDWR) == -1) {
                      		fprintf(stderr, "Cannot shut down client: %d socket\n", i);
                      	}
                      
                      	if (close(clients[cidx].sd) == -1) {
                      		fprintf(stderr, "Cannot close client: %d socket\n", i);
                      	}

		                clients[cidx].name = NULL;
		                clients[cidx].job_count = 0;
		                clients[cidx].sd = -1;
		                clients[cidx].last_ping = 0;

						break;
					 }
			    	case JOB: {
						int cidx = message->id;
						--clients[cidx].job_count;
						printf("Result: %s\n",
								message->content.char_val);
						break;
					}
					default:
						break;
			    }

			    free(peek_message);
			    free(message);
			}
		}
	}

	return NULL;
}

void* terminal_routine(void *args) {
	args = NULL;

	do {
	    char *filepath = NULL;
	    size_t n = 0;

		if (getline(&filepath, &n, stdin) == -1) {
			perror("Error getting line\n");
			free(filepath);
			continue;
		}

		filepath[strlen(filepath) -1] = '\0';

		if (!file_exists(filepath)) {
			perror("File does not exist\n");
			free(filepath);
			continue;
		}

		int cidx = 0;
		while (cidx < MAX_CLIENT_COUNT && clients[cidx].name == NULL) {
			++cidx;
		}
		if (cidx == MAX_CLIENT_COUNT) {
			perror("No clients registered\n");
			continue;
		}


	    FILE * fd = fopen(filepath, "r");
	    assert_file_ok(fd, filepath);

	    fseek(fd, 0, SEEK_END);
        long fsize = ftell(fd);
        fseek(fd, 0, SEEK_SET); 

	    char *buf = (char *)alloc(fsize);

	    fread(buf, fsize, 1, fd);
	    assert_file_ok(fd, filepath);

		cidx = 0;
		while (cidx < MAX_CLIENT_COUNT &&
				(clients[cidx].name == NULL || clients[cidx].job_count > 0)) {
			++cidx;
		}

		if (cidx == MAX_CLIENT_COUNT) {
			cidx = random() % MAX_CLIENT_COUNT;
		}

		while (clients[cidx].name == NULL) {
			cidx = (cidx + 1) % MAX_CLIENT_COUNT;
		}

		++clients[cidx].job_count;

		Message *job = create_message(fsize);

		job->type = JOB;
		job->id = request_id;
		strcpy(job->content.char_val, buf);

		++request_id;

		ssize_t sres = send(clients[cidx].sd, job,
				sizeof(Message) + job->length, 0);
		free(job);

		if (sres == -1) {
			perror("Error sending job request\n");
			continue;
		}
		printf("Sent job to client: %d, filepath: %s\n", cidx, filepath);

		free(filepath);
		free(buf);
	} while (1);

	return NULL;
}

		
int main(int argc, char **argv) {
	pthread_t ping_tid, listen_tid, terminal_tid;

	if (check_args(argc, argv) == -1) {
		exit(BAD_ARGS);
	}

    sigset_t set;
	sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    int retcode = sigprocmask(SIG_BLOCK, &set, NULL);
    if (retcode == -1) perror("sigprocmask");

	srand(time(NULL));

	sockname = argv[1];
	create_unix_socket(sockname);

	int port = (int)strtol(argv[2], NULL, 10);
	create_local_socket(port); 

	register_sighandlers();
	
	for (int i = 0; i < MAX_CLIENT_COUNT; ++i) {
		clients[i].name = NULL;
		clients[i].job_count = 0;
		clients[i].sd = -1;
		clients[i].last_ping = 0;
	}
	
	if (pthread_create(&ping_tid, NULL, ping_routine, NULL) == -1) {
		perror("Cannot create ping thread\n");
		exit(BAD_OP);
	}

	if (pthread_create(&listen_tid, NULL, listen_routine, NULL) == -1) {
		perror("Cannot create listen thread\n");
		exit(BAD_OP);
	}

	if (pthread_create(&terminal_tid, NULL, terminal_routine, NULL) == -1) {
		perror("Cannot create terminal thread\n");
		exit(BAD_OP);
	}

	void *retbuf;
	pthread_join(ping_tid, &retbuf);
	pthread_join(listen_tid, &retbuf);
	pthread_join(terminal_tid, &retbuf);

	return 0;
}


/**
 * Arguments:
 * unix socket path
 * port number
 **/
int check_args(int argc, char **argv) {
	if (argc != 3) {
		perror("Wrong arguments number. Expected 2\n");
		return -1;
	}

	if (!is_num(argv[2])) {
		perror("2rd argument - port number must be numeric!");
		return -1;
	}

	return 0;
}

int register_sighandlers() {
	if (atexit(cleaning_ladies) != 0) {
		perror("Cannot call cleaning ladies!\n");
		return -1;
	}

	return 0;
}

void cleaning_ladies() {
	if (close(epollfd) == -1) {
		perror("Error closing epoll fd\n");
	}
	
	if (shutdown(unix_sd, SHUT_RDWR) == -1) {
		perror("Cannot shut down unix socket\n");
	}

	if (close(unix_sd) == -1) {
		perror("Cannot close unix socket\n");
	}

	if (shutdown(local_sd, SHUT_RDWR) == -1) {
		perror("Cannot shut down local socket\n");
	}

	if (close(local_sd) == -1) {
		perror("Cannot close local socket\n");
	}

	if (remove(sockname) == -1) {
		perror("Error deleting socket file\n");
	}

	for (int i = 0; i < MAX_CLIENT_COUNT; ++i) {
		if (clients[i].name != NULL) {
			free(clients[i].name);
		}
	}

	free_unix_addr(unix_addr);
	free_local_addr(local_addr);
}

void create_unix_socket(char *sockpath) {
	ConnectionProps unix_connection;

	unix_connection.domain = AF_UNIX;
	unix_connection.sockpath = sockpath;

	unix_sd = socket(unix_connection.domain, CON_TYPE, 0);
	if (unix_sd == -1) {
		perror("Error creating unix socket\n");
		exit(BAD_OP);
	}

	unix_addr = get_sockaddr(unix_connection);
	if (unix_addr == NULL) {
		perror("Error gettig unix socket addres\n");
		exit(BAD_OP);
	}

	if (bind(unix_sd, unix_addr, sizeof(struct sockaddr_un)) == -1) {
		perror("Error binding unix socket\n");
		exit(BAD_OP);
	}

	if (listen(unix_sd, BACKLOG_SIZE) == -1) {
		perror("Error setting listen on unix socket");
		exit(BAD_OP);
	}

}

void create_local_socket(int port_number) {
	ConnectionProps local_connection;

	local_connection.domain = AF_INET;
	local_connection.port = port_number;
	strcpy(local_connection.ip, "127.0.0.1");

	local_sd = socket(local_connection.domain, CON_TYPE, 0);
	if (local_sd == -1) {
		perror("Error creating local socket\n");
		exit(BAD_OP);
	}

	local_addr = get_sockaddr(local_connection);
	if (local_addr == NULL) {
		perror("Error gettig local socket addres\n");
		exit(BAD_OP);
	}

	if (bind(local_sd, local_addr, sizeof(struct sockaddr_in)) == -1) {
		perror("Error binding local socket\n");
		exit(BAD_OP);
	}

	if (listen(local_sd, BACKLOG_SIZE) == -1) {
		perror("Error setting listen on local socket");
		exit(BAD_OP);
	}

}
