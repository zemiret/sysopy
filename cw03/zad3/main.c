#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <libgen.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "utils/utils.h"

#define ARCHIVE "archive"
#define COPY "copy"

struct Entry {
	char * monitoredFilepath;
	int timeout;
};

typedef struct ReadInfo {
	int fsize;
	char *buf;
} ReadInfo;


void check_args(int argc, char **argv);

void monitor(const char* listpath, time_t globalTimeout, const char* type, rlim_t cpulimit, rlim_t memlimit);

void monitor_file_archive(struct Entry monitorEntry, time_t globalTimeout);
void monitor_file_copy(struct Entry monitorEntry, time_t globalTimeout); 

ReadInfo read_file(const char *filepath);
struct stat statfile(const char *filepath); 
void assert_archive_exists();
int should_backup(struct stat oldStat, struct stat newStat);
char *get_archived_filename(char *filepath, time_t filetime);
void lock_file(FILE *fileh, const char *filename, int locktype);
void unlock_file(FILE *fileh, const char *filename);
void exec_copy(char *filepath, struct stat statbuf);

void print_usage_statistics();


int main(int argc, char** argv) {
	check_args(argc, argv);

	const char *listpath = argv[1];
	time_t timeout = (time_t)strtol(argv[2], NULL, 10);
	timeout = time(NULL) + timeout;
	const char* type = argv[3];
	rlim_t cpulimit = (rlim_t)strtol(argv[4], NULL, 10);
	rlim_t memlimit = (rlim_t)strtol(argv[5], NULL, 10);

	memlimit = memlimit * 1024 * 1024;	// get limit in MB

	monitor(listpath, timeout, type, cpulimit, memlimit);

	return 0;
}

/**
 * Args:
 * 1. list file pawth
 * 2. monitor time in seconds
 * 3. mode
 * 4. cpu time limit
 * 5. virtual memory limit
 * */
void check_args(int argc, char **argv) {
	if (argc != 6) {
        perror("Bad arguments count. Expected 5.\n");
		exit(-1);
	}

	if (!fileexists(argv[1])) {
		exit(-1);
	}

	if(strtol(argv[2], NULL, 10) == 0 && (errno == EINVAL || errno == ERANGE)) {
		perror("Second argument must be a number");
		exit(-1);
	}

	if (strcmp(argv[3], ARCHIVE) != 0 && strcmp(argv[3], COPY) != 0) {
		perror("Third argument must be 'archive' or 'copy'");
		exit(-1);
	}

	if(strtol(argv[4], NULL, 10) == 0 && (errno == EINVAL || errno == ERANGE)) {
		perror("Fourth argument must be a number");
		exit(-1);
	}

	if(strtol(argv[5], NULL, 10) == 0 && (errno == EINVAL || errno == ERANGE)) {
		perror("Fifth argument must be a number");
		exit(-1);
	}
}


void monitor(const char* listpath, time_t globalTimeout, const char* type, rlim_t cpulimit, rlim_t memlimit) {
	//struct MonitoredEntries entries = read_monitor_list(listpath);
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

		if (fork() == 0) {
			// in child
			fclose(fh);	// unless you do this, strange things can happen

			// set limits
			struct rlimit cpulimits;
			cpulimits.rlim_cur = cpulimits.rlim_max = cpulimit;
			struct rlimit memlimits;
			memlimits.rlim_cur = memlimits.rlim_max = memlimit;

			int resval;
			resval = setrlimit(RLIMIT_CPU, &cpulimits);
			if(resval == -1) {
				fprintf(stderr, "Setting cpulimit failed for process: %d", getpid());
			}
			resval = setrlimit(RLIMIT_AS, &memlimits);
			if(resval == -1) {
				fprintf(stderr, "Setting memlimit failed for process: %d", getpid());
			}

			struct Entry monitorEntry;
			monitorEntry.monitoredFilepath = watchfile;
			monitorEntry.timeout = timeout;
			
			if (strcmp(type, ARCHIVE) == 0) {
				monitor_file_archive(monitorEntry, globalTimeout);
			} else if (strcmp(type, COPY) == 0) {
				monitor_file_copy(monitorEntry, globalTimeout);
			} else {
	  			// in error case prevents dangling processes 
  				exit(-1);
			}
		} 
	}

	fclose(fh);

	pid_t child_pid;
	int status;

	while ((child_pid = wait(&status)) > 0) {
		if (WIFEXITED(status)) {
			int copy_count = WEXITSTATUS(status);
			printf("Proces %d utworzył %d kopii pliku\n", child_pid, copy_count); 
		}
	}
}


void monitor_file_archive(struct Entry monitorEntry, time_t globalTimeout) {
	int copy_count = 0;

	ReadInfo fileinfo = read_file(monitorEntry.monitoredFilepath);
	struct stat statbuf = statfile(monitorEntry.monitoredFilepath); 
	
	while(time(NULL) < globalTimeout) {
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

	free(fileinfo.buf);
	print_usage_statistics();
	exit(copy_count);
}

void monitor_file_copy(struct Entry monitorEntry, time_t globalTimeout) {
	int copy_count = 0;

	struct stat statbuf = statfile(monitorEntry.monitoredFilepath); 
	exec_copy(monitorEntry.monitoredFilepath, statbuf);
	
	while(time(NULL) < globalTimeout) {
		sleep(monitorEntry.timeout);	

		struct stat newStatbuf = statfile(monitorEntry.monitoredFilepath);

		if (should_backup(statbuf, newStatbuf)){
			exec_copy(monitorEntry.monitoredFilepath, newStatbuf); 

			statbuf = newStatbuf;
			++copy_count;
		}
	}

	print_usage_statistics();
	exit(copy_count);
}

void exec_copy(char *filepath, struct stat statbuf) {
	assert_archive_exists();			

	char *archivedFilepath = get_archived_filename(
			filepath, statbuf.st_mtim.tv_sec);

	if(fork() == 0) {
		execlp("cp", "cp", filepath, archivedFilepath, NULL);
		exit(0);
	}

	free(archivedFilepath);
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


void print_usage_statistics() {
	struct rusage usage;
	int res = getrusage(RUSAGE_SELF, &usage);
	if (res == -1) {
		fprintf(stderr, 
				"Cannot get usage statistics for process: %d\n", getpid());
	}

	printf("\n");

	printf("Usage statistics for process: %d\n", getpid()); 
	printf("user time: %ld μs\n", 
			usage.ru_utime.tv_sec * 1000000 + usage.ru_utime.tv_usec);
	printf("system time: %ld μs\n", 
			usage.ru_stime.tv_sec * 1000000 + usage.ru_stime.tv_usec);
	printf("max rss: %ld\n", usage.ru_maxrss);
	printf("page faults w/o IO: %ld\n", usage.ru_minflt);
	printf("page faults w IO: %ld\n", usage.ru_majflt);
	printf("total context switches: %ld\n", usage.ru_nvcsw + usage.ru_nivcsw);

	printf("\n");
}
