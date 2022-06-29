#include <stdio.h>

# ifndef UTILS_H
# define UTILS_H

void *alloc(const int SIZE);
void assert_file_ok(FILE* fh, const char* filename);
void assert_sys_file_ok(int res, const char *filename);

#endif
