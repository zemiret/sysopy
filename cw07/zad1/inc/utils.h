#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

# ifndef UTILS_H
# define UTILS_H

void *alloc(const int SIZE);

size_t count_in_string(const char *str, const char * sub);
size_t trim_string(char **out, char *str);

int is_empty(const char *s);
int is_num(const char *s);

char *get_date_str();
unsigned long long get_timestamp();

#endif
