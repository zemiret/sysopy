#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "../inc/utils.h"
#include "../inc/sigcommon.h"

#define SIGREG_ERR -198

int received_signals = 0;
pid_t sender_pid = -1;
char *send_type;

void check_args(int argc, char **argv);
void handle_signal(int signo, siginfo_t* siginfo, void *_);


int main(int argc, char** argv) {
	check_args(argc, argv);
	
	printf("Catcher pid %d\n", getpid());

	send_type = argv[1];

	register_signal_handlers(handle_signal);

	while(1) {
		pause();
	}
	
	return 0;
}

/**
 * Args:
 * 1. sending type
 * */
void check_args(int argc, char **argv) {
	if (argc != 2) {
        perror("Bad arguments count. Expected 1.\n");
		exit(-1);
	}

	const char *type = argv[1];
	if(!(
		  is_equal(type, KILL)
		  || is_equal(type, SIGQUEUE)
		  || is_equal(type, SIG_RT))) {
		fprintf(stderr, "3rd argument must be one of: %s, %s, %s\n",
				KILL,
				SIGQUEUE,
				SIG_RT);
	}
}


void handle_signal(int signo, siginfo_t* siginfo, void *_) {
	if (sender_pid == 0 || sender_pid == -1) {
		sender_pid = siginfo->si_pid;
	}

	if (signo == SIGUSR1 || signo == (SIGRTMIN+1)) {
		++received_signals;
	} else if (signo == SIGUSR2 || signo == (SIGRTMIN+2)) {
		send_signals_batch(sender_pid, received_signals, send_type);
		send_signal(sender_pid, SIGUSR2, send_type);
		exit(0);
	}
}
