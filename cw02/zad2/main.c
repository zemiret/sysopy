#include <stdbool.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "utils/utils.h"

bool check_args(int argc, char **argv);
void parse_args(int argc, char **argv);

char *get_entry_path(const char *dirpath, const char* entry_name);
void print_info(const char *path, struct stat stat_buf, struct dirent* dir_entry);
const char* get_typestr(struct dirent* dir_entry);

bool should_recur(struct dirent* dir_entry);
bool should_print(struct stat stat_buf, const char* code, const char* date);

#define LT "<"
#define EQ "="
#define GT ">"

void find_by_date(const char* dirpath, const char* code, const char *date);
struct dirent* read_next_entry(DIR *dh);

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

	if(dirpath == NULL) {
		fprintf(stderr, "Seems like some invalid path: %s\n", argv[1]);
		exit(errno);
	}

	find_by_date(dirpath, argv[2], argv[3]);
	free(dirpath);
}

void find_by_date(const char* dirpath, const char* code, const char *date) {
	DIR* dh = opendir(dirpath);
	if (dh == NULL) {
		fprintf(stderr, "Error opening dir: %s\n", dirpath);
		return;
	}

	struct dirent* dir_entry = read_next_entry(dh);
	
	while (dir_entry != NULL) {
		// Ignore links to . and .. 
		if (strcmp(dir_entry->d_name, ".") == 0 ||
				strcmp(dir_entry->d_name, "..") == 0) {
			dir_entry = read_next_entry(dh);
			continue;
		}

		const char *entry_name = dir_entry->d_name;
		char * entry_path = get_entry_path(dirpath, entry_name);
		
		struct stat stat_buf;
		stat(entry_path, &stat_buf);

		if(should_print(stat_buf, code, date)) {
			print_info(entry_path, stat_buf, dir_entry);
		}

		if (should_recur(dir_entry)) {
			find_by_date(entry_path, code, date);	
		}

		dir_entry = read_next_entry(dh);

		free(entry_path);
		entry_path = NULL;
	};

	int close_res = closedir(dh);
	if (close_res < 0) {
		fprintf(stderr, "Error closing direcotry: %s\n", dirpath);
		exit(errno);
	}
}

bool should_recur(struct dirent* dir_entry) {
	return 
		dir_entry->d_type != DT_LNK &&
		dir_entry->d_type == DT_DIR;
}

bool should_print(struct stat stat_buf, const char* code, const char* date) {
	long dateinsec = strtol(date, NULL, 10);

	if (strcmp(code, LT) == 0) {
		return stat_buf.st_mtim.tv_sec < dateinsec;
	} else if (strcmp(code, EQ) == 0) {
		return stat_buf.st_mtim.tv_sec == dateinsec;
	} else if (strcmp(code, GT) == 0) {
		return stat_buf.st_mtim.tv_sec > dateinsec;
	}

	return false;
}

char *get_entry_path(const char *dirpath, const char* entry_name) {
	char *entry_path = (char *)alloc((strlen(dirpath) + strlen(entry_name) + 2) * sizeof(char));
	strcpy(entry_path, dirpath);
	strcat(entry_path, "/");
	strcat(entry_path, entry_name);

	return entry_path;
}

void print_info(const char *path, struct stat stat_buf, struct dirent* dir_entry) {
	printf("\n");
	printf("Path: %s\n", path);
	printf("Type: %s\n", get_typestr(dir_entry));	
	printf("Byte size: %ld\n", stat_buf.st_size);	
	printf("Last access date: %ld\n", stat_buf.st_atim.tv_sec);
	printf("Last modification date: %ld\n", stat_buf.st_mtim.tv_sec);
}

const char* get_typestr(struct dirent* dir_entry) {
	switch(dir_entry->d_type) {
		case DT_BLK:
			return "Block device";
		case DT_CHR:
			return "Character device";
		case DT_DIR:
			return "Directory";
		case DT_FIFO:
			return "Named pipe (FIFO)";
		case DT_LNK:
			return "Symbolic link";
		case DT_REG:
			return "Regular file";
		case DT_SOCK:
			return "UNIX domain socket";
		case DT_UNKNOWN:
			return "Unknown";
		default:
			return "Unknown";
	}
}

struct dirent* read_next_entry(DIR *dh) {
	errno = 0;
	struct dirent* dir_entry = readdir(dh);

	if (dir_entry == NULL && errno != 0) {
		perror("Error reading directory");
		exit(errno);
	}

	return dir_entry;
}
