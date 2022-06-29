#include "utils.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

void *alloc(const int SIZE) {
	void *res = malloc(SIZE);
	if (res == NULL) {
		perror("Error allocating memory. WUT?\n");
		exit(1);
	}
	return res;
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
    // You need to free out
    char *end;
    size_t out_size;
	char *start = str;
  
    // Trim leading space
    while(isspace((unsigned char)*start)) start++;
  
    // Trim trailing space
    end = start + strlen(start) - 1;
    while(end > start && isspace((unsigned char)*end)) end--;
    end++;
	*end = '\0';

    // Set output size to minimum of trimmed string length and buffer size minus 1
    out_size = (end - start);

    // Copy trimmed string and add null terminator
	*out = (char *)alloc(out_size + 1);
	strcpy(*out, start);

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

char *get_date_str() {
	// You need to free!

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

	char *res = (char *)alloc(sizeof(char) * 19);
    
    sprintf(res, "%d-%d-%d %d:%d:%d",
			tm.tm_year + 1900,
			tm.tm_mon + 1,
			tm.tm_mday,
			tm.tm_hour,
			tm.tm_min,
			tm.tm_sec);

	return res;
}

unsigned long long get_timestamp() {
	struct timeval timer_usec;
    unsigned long long int timestamp_usec; /* timestamp in microsecond */
    if (!gettimeofday(&timer_usec, NULL)) {
      timestamp_usec = ((unsigned long long int) timer_usec.tv_sec) * 1000000ll +
                          (unsigned long long int) timer_usec.tv_usec;
    }
    else {
      timestamp_usec = -1;
    }

	return timestamp_usec;
}

int is_num(const char *str) {
	char *endptr;
	strtol(str, &endptr, 10);
	if (str == endptr) {
		return 0;
	}
	return 1;
}
