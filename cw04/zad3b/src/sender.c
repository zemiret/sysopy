#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "../inc/sigcommon.h"
#include "../inc/utils.h"

#define MASK_SET_ERR -199
#define SIGREG_ERR -198

int received_signals = 0;
int sig_count = 0;
int catcher_pid = -1;
char *send_type;
int received_response = 0;

void check_args(int argc, char **argv);
void handle_signal(int signo, siginfo_t *siginfo, void* _);

void pass_signal(int signo, siginfo_t *siginfo, void* _);

int main(int argc, char** argv) {
	check_args(argc, argv);

	catcher_pid = strtol(argv[1], NULL, 10);
	sig_count = strtol(argv[2], NULL, 10);
	send_type = argv[3];

	// registering dummy catcher so that signals do not kill this proc
	register_signal_handlers(pass_signal);
	
	// sending signals with waiting for the reply 
	for (int i = 0; i < sig_count; ++i) {
		received_response = 0;
		send_signal(catcher_pid, SIGUSR1, send_type);
		if (!received_response) {
			pause();
		}
	}

	register_signal_handlers(handle_signal);

	send_signal(catcher_pid, SIGUSR2, send_type);

	// registering receiving signal handler
	while(1) {
		pause();
	}

	return 0;
}

/**
 * Args:
 * 1. catcher pid
 * 2. signals count
 * 3. sending send_type
 * */
void check_args(int argc, char **argv) {
	if (argc != 4) {
        perror("Bad arguments count. Expected 3.\n");
		exit(-1);
	}
	if (!is_num(argv[1])) {
		perror("Catcher PID must be num!\n");
		exit(BAD_ARGS);
	}
	if (!is_num(argv[2])) {
		perror("Signals count must be number!\n");
		exit(BAD_ARGS);
	}
	const char *send_type = argv[3];
	if(!(
		  is_equal(send_type, KILL)
		  || is_equal(send_type, SIGQUEUE)
		  || is_equal(send_type, SIG_RT))) {
		fprintf(stderr, "3rd argument must be one of: %s, %s, %s\n",
				KILL,
				SIGQUEUE,
				SIG_RT);
	}
}

void handle_signal(int signo, siginfo_t *siginfo, void* _) {
	if (signo == SIGUSR1 || signo == (SIGRTMIN+1)) {
		++received_signals;
		send_signal(catcher_pid, SIGUSR1, send_type);
	} else if (signo == SIGUSR2 || signo == (SIGRTMIN+2)) {
		printf("Received %d signals\n", received_signals);
		printf("Should have received %d\n", sig_count); 
		exit(0);
	}
}

void pass_signal(int signo, siginfo_t *siginfo, void* _) {
	if (signo == SIGUSR1) {
		received_response = 1;
	}
}
