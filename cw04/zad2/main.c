#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <libgen.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "utils/utils.h"

struct Entry {
	char * monitoredFilepath;
	int timeout;
};

typedef struct ReadInfo {
	int fsize;
	char *buf;
} ReadInfo;

typedef struct ProcessMonitor {
	int pid;
	char * monitoredFilepath;
} ProcessMonitor;


int halt_monitoring = 0;
int copy_count = 0;

void check_args(int argc, char **argv);

void monitor(const char* listpath);
void monitor_file_archive(struct Entry monitorEntry);

ReadInfo read_file(const char *filepath);
struct stat statfile(const char *filepath); 
void assert_archive_exists();
int should_backup(struct stat oldStat, struct stat newStat);
char *get_archived_filename(char *filepath, time_t filetime);
void lock_file(FILE *fileh, const char *filename, int locktype);
void unlock_file(FILE *fileh, const char *filename);

void list_processes(ProcessMonitor *processMonitors, int monitor_count);
void read_and_handle_command(ProcessMonitor *processMonitors, int monitorsCount);

void register_signal_handlers();
void handle_signal(int signo);


int main(int argc, char** argv) {
	check_args(argc, argv);

	const char *listpath = argv[1];
	monitor(listpath);

	return 0;
}

/**
 * Args:
 * 1. list file pawth
 * */
void check_args(int argc, char **argv) {
	if (argc != 2) {
        perror("Bad arguments count. Expected 1.\n");
		exit(-1);
	}

	if (!fileexists(argv[1])) {
		exit(-1);
	}
}


void monitor(const char* listpath) {
	ProcessMonitor processMonitors[1024];
	int monitors_count = 0;

	char buf[512];
	FILE *fh = fopen(listpath, "r");
	assert_file_ok(fh, listpath);

	while(fgets(buf, sizeof buf, fh) != NULL) {
		char* watchfile = strtok(buf, " ");
		char* timeoutStr = strtok(NULL, " ");
		time_t timeout = (time_t)strtol(timeoutStr, NULL, 10);

		if (timeout == 0 && (errno == ERANGE || errno == EINVAL)) {
			perror("Incorrect timeout value in list file");
			exit(-1);
		}

		int child_pid = fork();

		if (child_pid == 0) {
			// in child
			fclose(fh);	// unless you do this, strange things can happen

			struct Entry monitorEntry;
			monitorEntry.monitoredFilepath = watchfile;
			monitorEntry.timeout = timeout;

			// register signal handlers
			register_signal_handlers();
			
			monitor_file_archive(monitorEntry);
		} else {
			// it's parent, let's remember the process
			ProcessMonitor p;

			p.monitoredFilepath = (char *)alloc(strlen(watchfile) + 1);
			strcpy(p.monitoredFilepath, watchfile);

			p.pid = child_pid;

			processMonitors[monitors_count] = p; 

			++monitors_count;
		}
	}
	// only parent gets here
	fclose(fh);

	// We have our processes in processMonitor array. 
	list_processes(processMonitors, monitors_count);
	read_and_handle_command(processMonitors, monitors_count);

	// Remember to free monitored filepaths!
	for (int i = 0; i < monitors_count; ++i) {
		free(processMonitors[i].monitoredFilepath);
	}
}


void monitor_file_archive(struct Entry monitorEntry) {
	ReadInfo fileinfo = read_file(monitorEntry.monitoredFilepath);
	struct stat statbuf = statfile(monitorEntry.monitoredFilepath); 
	
	while(1) {
		if (halt_monitoring) {
			continue;
		};
		sleep(monitorEntry.timeout);	

		struct stat newStatbuf = statfile(monitorEntry.monitoredFilepath);

		if (should_backup(statbuf, newStatbuf)){
			// copy file
			assert_archive_exists();			
			
			char *archivedFilename = get_archived_filename(
					monitorEntry.monitoredFilepath, newStatbuf.st_mtim.tv_sec);

			// save file
			FILE *archivedfh = fopen(archivedFilename, "wb+");

			lock_file(archivedfh, archivedFilename, LOCK_EX);

			assert_file_ok(archivedfh, archivedFilename);
			fwrite(fileinfo.buf, fileinfo.fsize, 1, archivedfh); 
			assert_file_ok(archivedfh, archivedFilename);
			unlock_file(archivedfh, archivedFilename);

			fclose(archivedfh);

			// free fbuf and save new file
			free(archivedFilename);
			free(fileinfo.buf);
			fileinfo.buf = NULL;
			fileinfo = read_file(monitorEntry.monitoredFilepath);

			// update stats
			statbuf = newStatbuf;
			++copy_count;
		}
	}
}

char *get_archived_filename(char *filepath, time_t filetime) {
	char *filepathCopy = (char *)alloc(strlen(filepath) + 1);
	strcpy(filepathCopy, filepath);

	char *fileBasename = basename(filepathCopy); 
	char *basepath = (char *)alloc(strlen("archiwum/") + strlen(fileBasename) + 1);

	strcpy(basepath, "archiwum/");
	strcat(basepath, fileBasename);

	struct tm *timestruct = localtime(&filetime);

	const int TIMESTR_SIZE = 20;
	
	char *timestr = (char *)alloc(TIMESTR_SIZE + 1);
	strftime(timestr, TIMESTR_SIZE + 1, "_%Y-%m-%d_%H-%M-%S", timestruct);

	char *res = (char *)alloc(strlen(basepath) + TIMESTR_SIZE + 1);
	strcpy(res, basepath);
	strcat(res, timestr);

    free(filepathCopy);
    free(basepath);
	free(timestr);

	return res;
}

int should_backup(struct stat oldStat, struct stat newStat) {
	if (newStat.st_mtim.tv_sec != oldStat.st_mtim.tv_sec) {
		return 1;
	}
	return 0;
}

void assert_archive_exists() {
	struct stat tsbuf;
	if (stat("archiwum", &tsbuf) == -1) {
	    mkdir("archiwum", 0751);
	}
}

struct stat statfile(const char *filepath) {
	struct stat statbuf;
	stat(filepath, &statbuf);

	return statbuf;
}

void lock_file(FILE *fileh, const char *filename, int locktype) {
	int archivedfd = fileno(fileh);
	int retval = flock(archivedfd, locktype);
	if (retval == -1) {
		fprintf(stderr, "Cannot lock file: %s, errno: %d\n", 
				filename, errno);
	}
}

void unlock_file(FILE *fileh, const char *filename) {
	int archivedfd = fileno(fileh);
	int retval = flock(archivedfd, LOCK_UN);
	if (retval == -1) {
		fprintf(stderr, "Cannot unlock file: %s, errno: %d\n", 
				filename, errno);
	}
}

ReadInfo read_file(const char *filepath) {
	// read file into memory
	FILE *fh = fopen(filepath, "rb");

	lock_file(fh, filepath, LOCK_SH);

    fseek(fh, 0, SEEK_END);
	assert_file_ok(fh, filepath);
	long fsize = ftell(fh);

    rewind(fh);
	assert_file_ok(fh, filepath);
    
	char* fbuf = (char *)alloc(fsize);
    fread(fbuf, fsize, 1, fh);
	assert_file_ok(fh, filepath);

	unlock_file(fh, filepath);

    fclose(fh);

	ReadInfo res;
	res.fsize = fsize;
	res.buf = fbuf;

	return res;
}


void list_processes(ProcessMonitor *processMonitors, int monitor_count) {
	for (int i = 0; i < monitor_count; ++i) {
		printf("Process %d is monitoring %s\n",
				processMonitors[i].pid,
				processMonitors[i].monitoredFilepath);
	}
}

void read_and_handle_command(ProcessMonitor *processMonitors, int monitorsCount) {
	char *cmd = NULL;
	size_t n;
	while(1) {
		n = 0;
		int charCount = getline(&cmd, &n, stdin);
		char *memoised_cmd = cmd;	
		if (charCount == -1) {
			printf("Error reading command\n");

			if (errno == EINVAL) { printf("EINVAL\n"); }
			if (errno == ENOMEM) { printf("ENOMEM\n"); }

			free(memoised_cmd);
			cmd = memoised_cmd = NULL;
			continue;
		}

		char *first_word = strsep(&cmd, " ");
		
		if (strcmp(first_word, "LIST\n") == 0) {
			list_processes(processMonitors, monitorsCount);
		} else if(strcmp(first_word, "STOP") == 0) {
			char *second_word = strsep(&cmd, " ");

			if(strcmp(second_word, "ALL\n") == 0) {
				for (int i = 0; i < monitorsCount; ++i) {
					kill(processMonitors[i].pid, SIGUSR1);
				}
			} else {
				int pid = (int)strtol(second_word, NULL, 10);
				if (pid == 0 && (errno == EINVAL || errno == ERANGE)) {
					printf("Incorrect PID value\n");
				}

				kill(pid, SIGUSR1);
			}

		} else if(strcmp(first_word, "START") == 0) {
			char *second_word = strsep(&cmd, " ");

			if(strcmp(second_word, "ALL\n") == 0) {
				for (int i = 0; i < monitorsCount; ++i) {
					kill(processMonitors[i].pid, SIGUSR2);
				}
			} else {
				int pid = (int)strtol(second_word, NULL, 10);
				if (pid == 0 && (errno == EINVAL || errno == ERANGE)) {
					printf("Incorrect PID value\n");
				}

				kill(pid, SIGUSR2);
			}
		} else if(strcmp(first_word, "END\n") == 0) {
			for (int i = 0; i < monitorsCount; ++i) {
				kill(processMonitors[i].pid, SIGTERM);
			}

        	pid_t child_pid;
        	int status;
        
        	while ((child_pid = wait(&status)) > 0) {
        		if (WIFSIGNALED(status) || WIFEXITED(status)) {
        			int child_copies = WEXITSTATUS(status);
        			printf("Proces %d utworzył %d kopii pliku\n", child_pid, child_copies); 
        		}
        	}

			free(memoised_cmd);
			cmd = memoised_cmd = NULL;
			break;
		} else {
			printf("Unrecognized command\n");
		}

		free(memoised_cmd);
		cmd = memoised_cmd = NULL;
	}
}

void register_signal_handlers() {
	signal(SIGUSR1, handle_signal);
	signal(SIGUSR2, handle_signal);
	signal(SIGTERM, handle_signal);
}

void handle_signal(int signo) {
	switch(signo) {
		case SIGUSR1:
			halt_monitoring = 1;
			break;
		case SIGUSR2:
			halt_monitoring = 0;
			break;
		case SIGTERM:
			exit(copy_count);
			break;
	}
}

