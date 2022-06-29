#include "utils.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>


void *alloc(const int SIZE) {
	void *res = malloc(SIZE);
	if (res == NULL) {
		perror("Error allocating memory.");
		exit(1);
	}
	return res;
}

void assert_file_ok(FILE* fh, const char* filename) {
    if (ferror(fh)) {
        fprintf(stderr, "Error working with file %s", filename);
        exit(-1);
    }
}

void assert_sys_file_ok(int res, const char *filename) {
    if (res < 0) {
        fprintf(stderr, "Error working with file %s", filename);
        exit(errno);
    }
}
