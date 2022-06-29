#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include "utils/utils.h"

void generate(const char* to_filename, const int RECORD_NUM, const int RECORD_SIZE);
void sys_sort(const char* filename, const int RECORD_NUM, const int RECORD_SIZE);
void lib_sort(const char* filename, const int RECORD_NUM, const int RECORD_SIZE);
void sys_copy(const char* from_filename, const char* to_filename, const int RECORD_NUM, const int BUFFER_SIZE);
void lib_copy(const char* from_filename, const char* to_filename, const int RECORD_NUM, const int BUFFER_SIZE);

bool check_args(int argc, char **argv);
void parse_args(int argc, char **argv);


int main(int argc, char** argv) {
    parse_args(argc, argv);
	return 0;
}


bool check_args(int argc, char **argv) {
	if (argc < 5) {
        perror("Bad arguments count.");
		return -1;
	}

	if ((strcmp(argv[1], "copy") != 0) &&
		(strcmp(argv[1], "sort") != 0) && 
		(strcmp(argv[1], "generate") != 0)) {

        perror("Unrecognized operation.");
		return -1;
	}

	if ((strcmp(argv[1], "copy") == 0) && argc != 7) {
        perror("Bad arguments count.");
		return -1;
	} else if ((strcmp(argv[1], "sort") == 0) && argc != 6) {
        perror("Bad arguments count.");
		return -1;
    } else if ((strcmp(argv[1], "generate") == 0) && argc != 5) {
        perror("Bad arguments count.");
		return -1;
    } 

	return 0;
}

void parse_args(int argc, char **argv) {
	if(check_args(argc, argv) != 0) {
		exit(-1);	
	};

    const char* opname = argv[1];

    if (strcmp(opname, "generate") == 0) {
        generate(argv[2], (int)strtol(argv[3], NULL, 10), (int)strtol(argv[4], NULL, 10));
        printf("%s\n", "Generated data.");
    } else if (strcmp(opname, "sort") == 0) {
        if (strcmp(argv[5], "lib") == 0) {
            lib_sort(argv[2], (int)strtol(argv[3], NULL, 10), (int)strtol(argv[4], NULL, 10));
        } else if (strcmp(argv[5], "sys") == 0) {
            sys_sort(argv[2], (int)strtol(argv[3], NULL, 10), (int)strtol(argv[4], NULL, 10));
        } 
    } else if (strcmp(opname, "copy") == 0) {
        if (strcmp(argv[6], "lib") == 0) {
            lib_copy(argv[2], argv[3], (int)strtol(argv[4], NULL, 10), (int)strtol(argv[5], NULL, 10));
        } else if (strcmp(argv[6], "sys") == 0) {
            sys_copy(argv[2], argv[3], (int)strtol(argv[4], NULL, 10), (int)strtol(argv[5], NULL, 10));
        }
    } 
}


void generate(const char* to_filename, const int RECORD_NUM, const int RECORD_SIZE) {
    FILE* to_fh = fopen(to_filename, "w");
    if (to_fh == NULL) {
        fprintf(stderr, "Cannot open file: %s for writing", to_filename);
        exit(-1);
    }

    FILE* random_fh = fopen("/dev/urandom", "r");
    if (random_fh == NULL) {
        perror("Cannot open /dev/urandom for reading");
        exit(-1);
    }

    void *buffer = alloc(RECORD_NUM * RECORD_SIZE);
    fread(buffer, RECORD_SIZE, RECORD_NUM, random_fh);
    fwrite(buffer, RECORD_SIZE, RECORD_NUM, to_fh);

    free(buffer);
    fclose(to_fh);
    fclose(random_fh);
}

void sys_sort(const char* filename, const int RECORD_NUM, const int RECORD_SIZE) {
    if (RECORD_SIZE == 0 || RECORD_NUM <= 1) {
        return;
    }

    int fd = open(filename, O_RDWR);
    assert_sys_file_ok(fd, filename);

    unsigned char *cur_min_buf = (unsigned char *)alloc(RECORD_SIZE);
    unsigned char *cur_buf = (unsigned char *)alloc(RECORD_SIZE);

    unsigned char *cur_min = (unsigned char *)alloc(1);
    unsigned char *cur = (unsigned char *)alloc(1);

    int res = 0;

    for (int i = 0; i < RECORD_NUM; ++i) {
        int min_pos = i;

        res = lseek(fd, i * RECORD_SIZE, SEEK_SET);
        assert_sys_file_ok(res, filename);
        res = read(fd, cur_min, 1);
        assert_sys_file_ok(res, filename);

        res = lseek(fd, i * RECORD_SIZE, SEEK_SET);
        assert_sys_file_ok(res, filename);
        res = read(fd, cur, 1);
        assert_sys_file_ok(res, filename);

        for (int j = i + 1; j < RECORD_NUM; ++j) {
			res = lseek(fd, j * RECORD_SIZE, SEEK_SET);
			assert_sys_file_ok(res, filename);
			res = read(fd, cur, 1);
			assert_sys_file_ok(res, filename);

            if (cur[0] < cur_min[0]) {
                min_pos = j;
				cur_min[0] = cur[0];
            }
        }

        // read into memory:
		res = lseek(fd, min_pos * RECORD_SIZE, SEEK_SET);
		assert_sys_file_ok(res, filename);
		res = read(fd, cur_min_buf, RECORD_SIZE);
		assert_sys_file_ok(res, filename);

		res = lseek(fd, i * RECORD_SIZE, SEEK_SET);
		assert_sys_file_ok(res, filename);
		res = read(fd, cur_buf, RECORD_SIZE);
		assert_sys_file_ok(res, filename);

        // swap:
		res = lseek(fd, i * RECORD_SIZE, SEEK_SET);
		assert_sys_file_ok(res, filename);
		res = write(fd, cur_min_buf, RECORD_SIZE);
		assert_sys_file_ok(res, filename);

		res = lseek(fd, min_pos * RECORD_SIZE, SEEK_SET);
		assert_sys_file_ok(res, filename);
		res = write(fd, cur_buf, RECORD_SIZE);
		assert_sys_file_ok(res, filename);
    }

    // clear memory
	res = close(fd);
	assert_sys_file_ok(res, filename);

    free(cur_min);
    free(cur);
    free(cur_min_buf);
    free(cur_buf);
}

void lib_sort(const char* filename, const int RECORD_NUM, const int RECORD_SIZE) {
    if (RECORD_SIZE == 0 || RECORD_NUM <= 1) {
        return;
    }

    FILE* fh = fopen(filename, "r+");

    if (fh == NULL) {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        exit(-1);
    }

    unsigned char *cur_min_buf = (unsigned char *)alloc(RECORD_SIZE);
    unsigned char *cur_buf = (unsigned char *)alloc(RECORD_SIZE);

    unsigned char *cur_min = (unsigned char *)alloc(1);
    unsigned char *cur = (unsigned char *)alloc(1);

    for (int i = 0; i < RECORD_NUM; ++i) {
        int min_pos = i;

        fseek(fh, i * RECORD_SIZE, SEEK_SET);
    	assert_file_ok(fh, filename);
        fread(cur_min, 1, 1, fh);
        assert_file_ok(fh, filename);

        fseek(fh, i * RECORD_SIZE, SEEK_SET);
    	assert_file_ok(fh, filename);
        fread(cur, 1, 1, fh);
        assert_file_ok(fh, filename);

        for (int j = i + 1; j < RECORD_NUM; ++j) {
            fseek(fh, j * RECORD_SIZE, SEEK_SET);
    		assert_file_ok(fh, filename);
            fread(cur, 1, 1, fh);
    		assert_file_ok(fh, filename);

            if (cur[0] < cur_min[0]) {
                min_pos = j;
				cur_min[0] = cur[0];
            }
        }

        // read into memory:
        fseek(fh, min_pos * RECORD_SIZE, SEEK_SET);
    	assert_file_ok(fh, filename);
        fread(cur_min_buf, RECORD_SIZE, 1, fh);
        assert_file_ok(fh, filename);

        fseek(fh, i * RECORD_SIZE, SEEK_SET);
    	assert_file_ok(fh, filename);
        fread(cur_buf, RECORD_SIZE, 1, fh);
        assert_file_ok(fh, filename);

        // swap:
        fseek(fh, i * RECORD_SIZE, SEEK_SET);
    	assert_file_ok(fh, filename);
        fwrite(cur_min_buf, RECORD_SIZE, 1, fh);
        assert_file_ok(fh, filename);

        fseek(fh, min_pos * RECORD_SIZE, SEEK_SET);
    	assert_file_ok(fh, filename);
        fwrite(cur_buf, RECORD_SIZE, 1, fh);
        assert_file_ok(fh, filename);
    }

    // clear memory
    fclose(fh);

    free(cur_min);
    free(cur);
    free(cur_min_buf);
    free(cur_buf);
}

void sys_copy(const char* from_filename, const char* to_filename, const int RECORD_NUM, const int BUFFER_SIZE) {
    if (BUFFER_SIZE <= 0 || RECORD_NUM <= 0) {
        return;
    }

	int from_fd = open(from_filename, O_RDONLY);
	if (from_fd == -1) {
        fprintf(stderr, "Cannot open file: %s\n", from_filename);
        exit(-1);
	}

	int to_fd = open(to_filename, O_WRONLY | O_CREAT, 0644);

	if (to_fd == -1) {
        fprintf(stderr, "Cannot open file: %s\n", to_filename);
        exit(-1);
	}

    unsigned char* buf = (unsigned char *)alloc(BUFFER_SIZE);
	int res = 0;

    for (int i = 0; i < RECORD_NUM; ++i) {
        // read to buffer
		res = read(from_fd, buf, BUFFER_SIZE);
		assert_sys_file_ok(res, from_filename);

        // write to file 
		res = write(to_fd, buf, BUFFER_SIZE);
		assert_sys_file_ok(res, from_filename);
    }

	res = close(from_fd);
	assert_sys_file_ok(res, from_filename);
	res = close(to_fd);
	assert_sys_file_ok(res, to_filename);

	free(buf);
}

void lib_copy(const char* from_filename, const char* to_filename, const int RECORD_NUM, const int BUFFER_SIZE) {
    if (BUFFER_SIZE <= 0 || RECORD_NUM <= 0) {
        return;
    }

    FILE* from_fh = fopen(from_filename, "r");
    if (from_fh == NULL) {
        fprintf(stderr, "Cannot open file: %s\n", from_filename);
        exit(-1);
    }

    FILE* to_fh = fopen(to_filename, "w");
    if (to_fh == NULL) {
        fprintf(stderr, "Cannot open file: %s\n", from_filename);
        exit(-1);
    }

    unsigned char* buf = (unsigned char *)alloc(BUFFER_SIZE);

    for (int i = 0; i < RECORD_NUM; ++i) {
        // read to buffer
        fread(buf, BUFFER_SIZE, 1, from_fh);
        assert_file_ok(from_fh, from_filename);

        // write to file 
        fwrite(buf, BUFFER_SIZE, 1, to_fh);
        assert_file_ok(to_fh, to_filename);
    }

    free(buf);

    fclose(from_fh);
    fclose(to_fh);
}
