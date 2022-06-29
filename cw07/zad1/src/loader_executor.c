#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../inc/config.h"
#include "../inc/utils.h"


int check_args(int argc, char **argv);

int main(int argc, char **argv) {
	if (check_args(argc, argv) == -1) {
		exit(BAD_ARGS);
	}

	const char* workers_count = argv[1];
	const char* max_box_mass = argv[2];
	const char* cycles_count = "-1";
	if (argc == 4) {
		cycles_count = argv[3];
	}

	int worksers_count_numeric = (int)strtol(workers_count, NULL, 10); 
	int max_box_mass_numeric = (int)strtol(max_box_mass, NULL, 10); 

	for (int i = 0; i < worksers_count_numeric; ++i) {
		if (fork() == 0) {
			// child
			char cur_box_mass[36];	
			sprintf(cur_box_mass, "%d", max_box_mass_numeric / (i + 1));

			execl("./loader.out", "./loader.out",
					cur_box_mass,
					cycles_count,
					NULL);
		}
	}

	int status;
	while(wait(&status) != -1) {
	}

	return 0;
}

/*
 * Arguments
 * 1. workers count 
 * 2. max box mass
 * 3. [optional] cycles count
 */
int check_args(int argc, char **argv) {
	if (argc != 3 && argc != 4) {
		perror("Bad argument count. Expected 2 or 3");
		return -1;
	}
	
	if (!is_num(argv[1]) || 
			!is_num(argv[2]) ||
			(argc == 4 && !is_num(argv[3]))) {
		perror("Arguments must be numeric!");
		return -1;
	}

	return 0;
}
