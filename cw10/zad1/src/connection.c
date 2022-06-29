#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../inc/connection.h"
#include "../inc/utils.h"


int parse_connection(const char *str) {
	if (strcmp(str, "UNIX") == 0) {
		return AF_UNIX;
	} else if(strcmp(str, "LOCAL") == 0) {
		return AF_INET;
	} else {
		return -1;
	}
}	

	// Remember to free the result!

struct sockaddr* get_sockaddr(ConnectionProps connection) {
	if (connection.domain == AF_UNIX) {
		struct sockaddr_un* addr =
			(struct sockaddr_un *)alloc(sizeof(struct sockaddr_un));
		addr->sun_family = AF_UNIX;
		strcpy(addr->sun_path, connection.sockpath);

		return (struct sockaddr* )addr;
	} else if (connection.domain == AF_INET) {
		struct in_addr ip_addr;
		ip_addr.s_addr = inet_addr(connection.ip);

		struct sockaddr_in* addr =
			(struct sockaddr_in *)alloc(sizeof(struct sockaddr_in));
		addr->sin_family = AF_INET;
		addr->sin_port = htobe16(connection.port);
		addr->sin_addr = ip_addr;

		return (struct sockaddr *)addr;
	} else {
		return NULL;
	}
}

void free_unix_addr(struct sockaddr* addr) {
	struct sockaddr_un* unaddr = (struct sockaddr_un *)addr;
	free(unaddr);
}

void free_local_addr(struct sockaddr* addr) {
	struct sockaddr_in* inaddr = (struct sockaddr_in *)addr;
	free(inaddr);
}

Message* create_message(int data_len) {
	// Remember about free!
	Message* res = (Message *)alloc(sizeof(Message) + data_len);
	res->length = data_len - 1;
	res->id = -1;
	res->content.char_val = (char *)res + sizeof(Message);

	return res;
}
