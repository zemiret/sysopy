#ifndef CONNECTION_H
#define CONNECTION_H

typedef struct {
	int domain;
	char *name;
	char ip[16];
	char *sockpath;
	int port;
} ConnectionProps;

typedef enum {
	LOGIN = 1, LOGOUT, PING, JOB
} MessageType;

typedef enum {
	OK, FAILED
} LoginStatus;

typedef union {
	char *char_val;
	LoginStatus* int_val;
} MessageContent;

typedef struct {
	MessageType type;
	int id;
	int length;
	MessageContent content;
} Message;

int parse_connection(const char *str);

struct sockaddr* get_sockaddr(ConnectionProps connection);
void free_unix_addr(struct sockaddr* addr);
void free_local_addr(struct sockaddr* addr);

Message* create_message(int data_len);

#endif

