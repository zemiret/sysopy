#include "utils.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


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

int get_line_count(FILE *fh) {
	int line_count = 0;

	for (char c = getc(fh); c != EOF; c = getc(fh)) {
		if (c == '\n') {
			++line_count;
		}
	}

	rewind(fh);
	return line_count;
}

int fileexists(const char * filename){
    /* try to open file to read */
    FILE *file = fopen(filename, "r");
    if (file){
        fclose(file);
        return 1;
    }
    return 0;
}

size_t count_in_string(const char *str, const char *sub) {
    int N = strlen(str);
    int M = strlen(sub);
    int res = 0;

    /* A loop to slide pat[] one by one */
    for (int i = 0; i <= N - M; i++)
    {
        /* For current index i, check for
           pattern match */
        int j;
        for (j = 0; j < M; j++)
            if (str[i+j] != sub[j])
                break;

        // if pat[0...M-1] = txt[i, i+1, ...i+M-1]
        if (j == M)
        {
           res++;
           j = 0;
        }
    }
    return res;
}

size_t trim_string(char **out, char *str) {
    // Keep in mind, dealloates str and allocates out
  
    char *end;

    size_t out_size;
  
    // Trim leading space
    while(isspace((unsigned char)*str)) str++;
  
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end++;
	*end = '\0';

    // Set output size to minimum of trimmed string length and buffer size minus 1
    out_size = (end - str);

    // Copy trimmed string and add null terminator
	*out = (char *)alloc(out_size + 1);
	strcpy(*out, str);

    return out_size;
}

int is_empty(const char *s) {
	const char * const mem = s;  
    while (*s != '\0') {
      if (!isspace((unsigned char)*s)) {
		s = mem;
		return 0;
	  }
      s++;
    }
    return 1;
}

int is_num(const char *str) {
	if(strtol(str, NULL, 10) == 0 && 
			(errno == EINVAL || errno == ERANGE)) {
		return 0;
	}
	return 1;
}
