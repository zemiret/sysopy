#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../inc/pgm.h"
#include "../inc/matrix.h"
#include "../inc/utils.h"

Matrix * read_pgm(const char *filename) {
	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		fprintf(stderr, "Cannot read file %s\n", filename);
		return NULL;
	}

	int res = 0;
	char buf[128];
	char * lineptr = NULL;
	size_t n = 0;
	ssize_t getline_res;


	getline_res = getline(&lineptr, &n, f);
	if (getline_res == -1) {
		free(lineptr);
		fprintf(stderr, "Error reading 1st line from file %s\n", filename); 
		return NULL;
	}
	res = sscanf(lineptr, "%s\n", buf);
	free(lineptr);
	if (res == EOF || strcmp(buf, "P2") != 0) {
		fprintf(stderr, "Malformed file %s\n", filename);
		return NULL;
	}


	lineptr = NULL;
	n = 0;
	getline_res = getline(&lineptr, &n, f);
	if (getline_res == -1) {
		free(lineptr);
		fprintf(stderr, "Error reading 2nd line from file %s\n", filename); 
		return NULL;
	}
	int w, h, m; 
	res = sscanf(lineptr, "%d %d\n", &w, &h);
	free(lineptr);

	lineptr = NULL;
	n = 0;
	getline_res = getline(&lineptr, &n, f);
	if (getline_res == -1) {
		free(lineptr);
		fprintf(stderr, "Error reading 3rd line from file %s\n", filename); 
		return NULL;
	}
	res = sscanf(lineptr, "%d\n", &m);
	free(lineptr);

	if (res == EOF || w < 0 || h < 0 || m < 0 || m > 255) {
		fprintf(stderr, "Malformed file %s\n", filename);
		return NULL;
	}

	Matrix *pgm_matrix = init_matrix(h, w); 
	short int c;	

	long cur_pos = ftell(f);
	fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, cur_pos, SEEK_SET);
    
    char *content = (char *)alloc(fsize - cur_pos + 1);
    fread(content, 1, fsize, f);

	char *tok = strtok(content, " \t\n");
	char *endptr;

	c = (short int)strtol(tok, &endptr, 10);
	if (tok == endptr || c < 0 || c > m) {
		fprintf(stderr, "Bad pixel range. File %s\n", filename); 
		return NULL;
	}

	pgm_matrix->arr[0][0] = c;

	for (int i = 0; i < h * w; ++i) {
		tok = strtok(NULL, " \t\n");

		if (tok == NULL) {
			break;
		}

		c = (short int)strtol(tok, &endptr, 10);
	    if (tok == endptr || c < 0 || c > m) {
			fprintf(stderr, "Bad pixel range. File %s\n", filename); 
	    	return NULL;
		}

		pgm_matrix->arr[i / w][i % w] = c;
	}

	fclose(f);
	free(content);

	return pgm_matrix;
}

MatrixFloat * read_filter(const char *filename) {
	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		fprintf(stderr, "Cannot read file %s\n", filename);
		return NULL;
	}

	int res = 0;
	char buf[128];
	char * lineptr = NULL;
	size_t n = 0;
	ssize_t getline_res;


	getline_res = getline(&lineptr, &n, f);
	if (getline_res == -1) {
		free(lineptr);
		fprintf(stderr, "Error reading 1st line from file %s\n", filename); 
		return NULL;
	}
	res = sscanf(lineptr, "%s\n", buf);
	free(lineptr);
	if (res == EOF || strcmp(buf, "P2") != 0) {
		fprintf(stderr, "Malformed file %s\n", filename);
		return NULL;
	}


	lineptr = NULL;
	n = 0;
	getline_res = getline(&lineptr, &n, f);
	if (getline_res == -1) {
		free(lineptr);
		fprintf(stderr, "Error reading 2nd line from file %s\n", filename); 
		return NULL;
	}
	int w, h, m; 
	res = sscanf(lineptr, "%d %d\n", &w, &h);
	free(lineptr);

	lineptr = NULL;
	n = 0;
	getline_res = getline(&lineptr, &n, f);
	if (getline_res == -1) {
		free(lineptr);
		fprintf(stderr, "Error reading 3rd line from file %s\n", filename); 
		return NULL;
	}
	res = sscanf(lineptr, "%d\n", &m);
	free(lineptr);

	if (res == EOF || w < 0 || h < 0 || m < 0 || m > 255) {
		fprintf(stderr, "Malformed file %s\n", filename);
		return NULL;
	}

	MatrixFloat *pgm_matrix = init_float_matrix(h, w); 

	long cur_pos = ftell(f);
	fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, cur_pos, SEEK_SET);
    
    char *content = (char *)alloc(fsize - cur_pos + 1);
    fread(content, 1, fsize, f);

	char *tok = strtok(content, " \t\n");
	char *endptr;

	float c = strtof(tok, &endptr);
	if (tok == endptr || c < 0 || c > m) {
		fprintf(stderr, "Bad pixel range. File %s\n", filename); 
		return NULL;
	}

	pgm_matrix->arr[0][0] = c;

	for (int i = 0; i < h * w; ++i) {
		tok = strtok(NULL, " \t\n");

		if (tok == NULL) {
			break;
		}

		c = (float)strtof(tok, &endptr);
	    if (tok == endptr || c < 0 || c > m) {
			fprintf(stderr, "Bad pixel range. File %s\n", filename); 
	    	return NULL;
		}

		pgm_matrix->arr[i / w][i % w] = c;
	}

	fclose(f);
	free(content);

	return pgm_matrix;
}
