#include "../inc/sigcommon.h"
#include "../inc/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void send_signal(int proc_pid, int signo, const char *type) {
	if (is_equal(type, KILL)) {
		if(kill(proc_pid, signo) != 0) {
			perror("Error sending - kill\n");	
		}
	} else if (is_equal(type, SIGQUEUE)) {
		union sigval sv;
		if(sigqueue(proc_pid, signo, sv) != 0) {
			perror("Error sending - sigqueue\n");
		}
	} else if (is_equal(type, SIG_RT)) {
		int rtsigno;
		if (signo == SIGUSR1) {
			rtsigno = SIGRTMIN + 1;
		} else {
			rtsigno = SIGRTMIN + 2;
		}

		if(kill(proc_pid, rtsigno) != 0) {
			perror("Error sending - realtime kill\n");	
		}
	}
}

void register_signal_handlers(void (*handle_signal)(int, siginfo_t *, void *)) {
	sigset_t mask;
	sigfillset(&mask);
	sigdelset(&mask, SIGUSR1);
	sigdelset(&mask, SIGUSR2);

	struct sigaction handler;
	handler.sa_flags = SA_SIGINFO;
	handler.sa_sigaction = handle_signal;
	handler.sa_mask = mask;

	sigaction(SIGRTMIN+1, &handler, NULL);
	sigaction(SIGRTMIN+2, &handler, NULL);
 	sigaction(SIGUSR1, &handler, NULL);
 	sigaction(SIGUSR2, &handler, NULL);
}
