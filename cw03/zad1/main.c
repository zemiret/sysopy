#include <stdbool.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
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

void find_by_date(const char* dirpath, const char* code, const char *date, const char * relpath);
struct dirent* read_next_entry(DIR *dh);

int main(int argc, char** argv) {
    parse_args(argc, argv);
	exit(0);
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

	find_by_date(dirpath, argv[2], argv[3], "");
	free(dirpath);
}

void find_by_date(const char* dirpath, const char* code, const char *date, const char * relpath) {
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

		if (should_recur(dir_entry)) {
			char * next_rel_path = get_entry_path(relpath, entry_name); 

			pid_t child_pid = fork();

			if (child_pid == 0) {	// parent
				printf("Child pid: %d\n", getpid());
				printf("Path: %s\n", next_rel_path);
				execlp("ls", "ls", "-l", entry_path, NULL);
				exit(0);
			}

			waitpid(child_pid, NULL, WUNTRACED);

			find_by_date(entry_path, code, date, next_rel_path);

			free(next_rel_path);
			next_rel_path = NULL;
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

char *get_entry_path(const char *dirpath, const char* entry_name) {
	char *entry_path = (char *)alloc((strlen(dirpath) + strlen(entry_name) + 2) * sizeof(char));
	strcpy(entry_path, dirpath);
	strcat(entry_path, "/");
	strcat(entry_path, entry_name);

	return entry_path;
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
