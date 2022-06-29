#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "utils/utils.h"

void check_args(int argc, char **argv);

int main(int argc, char **argv) {
	check_args(argc, argv);

	if(mkfifo(argv[1], S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
		perror("Error creating fifo");
		exit(-1);
	}

	FILE *fh = fopen(argv[1], "r");
	assert_file_ok(fh, argv[1]);
	while(1) {
		char *line = NULL;
		size_t n = 0;

		if (getline(&line, &n, fh) != -1) {
			printf("%s", line);
		}

		free(line);
	}

	fclose(fh);
}

void check_args(int argc, char **_) {
	if (argc != 2) {
		perror("master expects 1 argument\n");
		exit(-1);
	}
}

