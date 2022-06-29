#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/random.h>
#include <sys/file.h>
#include "utils/utils.h"


int main(int argc, char **argv) {
	srand(time(NULL));

	if (argc != 5) {
		perror("Expected 4 arguments");
		exit(-1);
	}
	
	const char *filepath = argv[1];
	int pmin = (int)strtol(argv[2], NULL, 10);
	int pmax = (int)strtol(argv[3], NULL, 10);
	int bytes = (int)strtol(argv[4], NULL, 10);
	
	while(1) {
		int sleeptime = (rand() % (pmax - pmin)) + pmin;	
		sleep(sleeptime);

		FILE *fh = fopen(filepath, "ab");
		assert_file_ok(fh, filepath);

		int pid = getpid();
		int timenow = time(NULL);

		const int MAXLEN = 128 + bytes;
		char buf[MAXLEN];

		int bufpos = snprintf(buf, MAXLEN, "%d %d %d", pid, sleeptime, timenow);
		getrandom(buf + bufpos, bytes, 0); 		

		int fd = fileno(fh);
		int lockres = flock(fd, LOCK_EX);
		if (lockres == -1) {
			continue;
		}
		fwrite(buf, MAXLEN, 1, fh);
		assert_file_ok(fh, filepath);
		flock(fd, LOCK_UN);

		fclose(fh);
	}

	return 0;
}
