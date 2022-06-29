#ifndef CONFIG_H
#define CONFIG_H

#define MAX_CLIENTS 32 
#define MAX_MSG_LEN 256

#define SERVER_QUEUE_ID "/sysop_server_queue"
#define BASE_QUEUE_ID "/sysop_client_queue" 

#define REQUEST_LEN sizeof(Request) + 2
#define RESPONSE_LEN sizeof(Response) + 1

#define MAX_PRIO 4
#define HIGH_PRIO 3
#define MID_PRIO 2
#define LOW_PRIO 1
#define PRIO_MSGTYPE -100

#endif
