#include "../inc/utils.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


void *alloc(const int SIZE) {
	void *res = malloc(SIZE);
	if (res == NULL) {
		perror("Error allocating memory.");
		exit(1);
	}
	return res;
}

int is_num(const char *str) {
	if(strtol(str, NULL, 10) == 0 && 
			(errno == EINVAL || errno == ERANGE)) {
		return 0;
	}
	return 1;
}

int is_equal(const char* str1, const char *str2) {
	return strcmp(str1, str2) == 0;
}
