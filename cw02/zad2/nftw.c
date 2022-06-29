#define _XOPEN_SOURCE 500

#include <stdbool.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ftw.h>
#include "utils/utils.h"

bool check_args(int argc, char **argv);
void parse_args(int argc, char **argv);

void print_info(const char *path, struct stat stat_buf, int typeflag);
const char* get_typestr(int typeflag);
int traverse(const char* filepath, const struct stat *stat_buf, int typeflag, struct FTW* ftwbif);

bool should_print(const struct stat* stat_buf);

#define LT "<"
#define EQ "="
#define GT ">"

void find_by_date(const char* dirpath);

char* code;
char* date;

int main(int argc, char** argv) {
    parse_args(argc, argv);
	return 0;
}


bool check_args(int argc, char **argv) {
	if (argc != 4) {
        perror("Bad arguments count.\n");
		return -1;
	}

	if (strcmp(argv[2], LT) != 0 && 
	    strcmp(argv[2], EQ) != 0 && 
		strcmp(argv[2], GT) != 0) {

        fprintf(stderr, "Second argument must be one of %s, %s, %s.\n", LT, EQ, GT);
		return -1;
	}

	return 0;
}

void parse_args(int argc, char **argv) {
	if(check_args(argc, argv) != 0) {
		exit(-1);	
	}
	
	char* dirpath = realpath(argv[1], NULL);
	code = argv[2];
	date = argv[3];

	if(dirpath == NULL) {
		fprintf(stderr, "Seems like some invalid path: %s\n", argv[1]);
		exit(errno);
	}

	find_by_date(dirpath);
	free(dirpath);
}

int traverse(const char* filepath, const struct stat *stat_buf, int typeflag, struct FTW* ftwbif) {
	if(should_print(stat_buf)) {
		print_info(filepath, *stat_buf, typeflag);  		
	}

	return 0;
}

void find_by_date(const char* dirpath) {
	int res = nftw(dirpath, &traverse, 50, FTW_PHYS);
	if (res < 0) {
		fprintf(stderr, "Error traversing: %s\n", dirpath);
		exit(-1);
	}
}


bool should_print(const struct stat* stat_buf) {
	long dateinsec = strtol(date, NULL, 10);

	if (strcmp(code, LT) == 0) {
		return stat_buf->st_mtim.tv_sec < dateinsec;
	} else if (strcmp(code, EQ) == 0) {
		return stat_buf->st_mtim.tv_sec == dateinsec;
	} else if (strcmp(code, GT) == 0) {
		return stat_buf->st_mtim.tv_sec > dateinsec;
	}

	return false;
}

void print_info(const char *path, struct stat stat_buf, int typeflag) {
	printf("\n");
	printf("Path: %s\n", path);
	printf("Type: %s\n", get_typestr(typeflag));	
	printf("Byte size: %ld\n", stat_buf.st_size);	
	printf("Last access date: %ld\n", stat_buf.st_atim.tv_sec);
	printf("Last modification date: %ld\n", stat_buf.st_mtim.tv_sec);
}

const char* get_typestr(int typeflag) {
	switch(typeflag) {
		case FTW_D:
		case FTW_DP:
		case FTW_DNR:
			return "Directory";
		case FTW_SL:
			return "Symbolic link";
		case FTW_SLN:
			return "Nonexistent symbolic link";
		case FTW_F:
			return "Regular file";
		default:
			return "Unknown";
	}
}

