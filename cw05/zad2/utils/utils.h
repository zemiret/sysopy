#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

# ifndef UTILS_H
# define UTILS_H

void *alloc(const int SIZE);

void assert_file_ok(FILE* fh, const char* filename);
void assert_sys_file_ok(int res, const char *filename);
int get_line_count(FILE *fh);
int fileexists(const char * filename);

size_t count_in_string(const char *str, const char * sub);
size_t trim_string(char **out, char *str);
int is_empty(const char *s);

int is_num(const char *str);

#endif
