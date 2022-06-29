#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

void handle_sig_stp(int);
void handle_sig_int(int);
void print_time();
void loop_timer(int);

int is_paused;


int main() {
	struct sigaction sig_handle;
	sig_handle.sa_handler = handle_sig_int;

	if(sigaction(SIGINT, &sig_handle, NULL) == -1) {
		puts("Couldn't register SIGINT signal\n");
		exit(-1);
	}
	if(signal(SIGTSTP, handle_sig_stp) == SIG_ERR) {
		puts("Couldn't register SIGSTP signal\n");
		exit(-1);
	}

	is_paused = 0;

	while(1) {
		print_time();		
		sleep(1);
		if (is_paused) {
			pause();
		}
	}

	return 0;
}

void handle_sig_stp(int _) {
	if (!is_paused) {
		puts("Oczekuję na CTRL+Z - kontynuacja albo CTRL+C - zakończenie programu\n");
		is_paused = 1;
	} else {
		is_paused = 0;
	}
}

void handle_sig_int(int _) {
	puts("Odebrano sygnał SIGINT");
	exit(0);
}

void print_time() {
  time_t rawtime;

  time(&rawtime);
  printf("Current local time and date: %s", ctime(&rawtime));
}

