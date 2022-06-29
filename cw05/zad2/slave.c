#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "utils/utils.h"

void check_args(int argc, char **argv);

int main(int argc, char **argv) {
	check_args(argc, argv);
	srand(time(NULL));

	char *fifopath = argv[1];

	int pid = getpid();
	printf("Pid: %d\n", pid);

	int N = (int)strtol(argv[2], NULL, 10);

	for (int i = 0; i < N; ++i) {
		FILE *fh = fopen(fifopath, "w");
		assert_file_ok(fh, fifopath);
		FILE *date_stream = popen("date", "r");

		char *line = NULL;
		size_t n = 0;

		if (getline(&line, &n, date_stream) != -1) {
			fprintf(fh, "%d %s", pid, line);
		}

		sleep(rand() % 3 + 2);

		free(line);
		pclose(date_stream);
		fclose(fh);
	}
}

void check_args(int argc, char **argv) {
	if (argc != 3) {
		perror("slave expects 2 arguments\n");
		exit(-1);
	}

	if (!is_num(argv[2])) { 
		perror("Second argument must be a number\n");
		exit(-1);
	}
}

