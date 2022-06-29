#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

void handle_sig_stp(int);
void handle_sig_int(int);

int child_pid;


int main() {
	child_pid = fork();

	if (child_pid == -1) {
		perror("Error creating child process\n");
		exit(errno);
	}
	
	if (child_pid == 0) {
		// it's child
		execl("./printtime.sh", "printtime", NULL);
	} else {
		// it's parent
    	struct sigaction sig_handle;
    	sig_handle.sa_handler = handle_sig_int;
    
    	if(sigaction(SIGINT, &sig_handle, NULL) == -1) {
    		puts("Couldn't register SIGINT signal\n");
    		exit(-1);
			kill(child_pid, SIGTERM);
    	}
    	if(signal(SIGTSTP, handle_sig_stp) == SIG_ERR) {
    		puts("Couldn't register SIGSTP signal\n");
    		exit(-1);
			kill(child_pid, SIGTERM);
    	}

		while(1) {
			pause();
		}
	}

	return 0;
}

void handle_sig_stp(int _) {
	if (child_pid == -1) {	// -1 means was dead
    	child_pid = fork();
    
    	if (child_pid == -1) {
    		perror("Error creating child process\n");
    		exit(errno);
    	}
    	
    	if (child_pid == 0) {
    		// it's child
    		execl("./printtime.sh", "printtime", NULL);
    	}
	} else {
		puts("Oczekuję na CTRL+Z - kontynuacja albo CTRL+C - zakończenie programu\n");
		kill(child_pid, SIGTERM);
		child_pid = -1;
	}
}

void handle_sig_int(int _) {
	if (child_pid != -1) {
		kill(child_pid, SIGTERM);
		child_pid = -1;
	}
	puts("Odebrano sygnał SIGINT");
	exit(0);
}

