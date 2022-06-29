#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "../inc/config.h"
#include "../inc/utils.h"
#include "../inc/matrix.h"
#include "../inc/pgm.h"

typedef enum Optype { BLOCK, INTERLEAVED } Optype;

typedef struct {
	Matrix *image;
	MatrixFloat *filter;
	Matrix *out;
	int k;
	int m;
	Optype optype;
} FilterArgs;

#ifndef max
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef min
	#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif


int check_args(int argc, char **argv);
void* filter(void *args);
void filter_col(Matrix* image, MatrixFloat* filter, Matrix* out, int col);
void save_out(Matrix * out, const char *outpath);

int main(int argc, char **argv) {
	if (check_args(argc, argv) == -1) {
		exit(BAD_ARGS);
	}

	int thread_count = (int)strtol(argv[1], NULL, 10);
	const char *out_filename = argv[5]; 

	Optype optype;
	if (strcmp(argv[2], "block") == 0) {
		optype = BLOCK;
	} else {
		optype = INTERLEAVED;
	}

	Matrix *pgm = read_pgm(argv[3]); 
	MatrixFloat *filter_matrix = read_filter(argv[4]); 
	if (pgm == NULL || filter_matrix == NULL) {
		perror("Error reading file\n");
		exit(MALFORMED_FILE);
	}

	Matrix *out_matrix = init_matrix(pgm->rows, pgm->cols);

	for(int i = 0; i < out_matrix->rows; ++i) {
		for (int j = 0; j < out_matrix->cols; ++j) {
			out_matrix->arr[i][j] = -1;
		}
	}

	FilterArgs** call_args = (FilterArgs **)alloc(sizeof(FilterArgs*) * thread_count);
	for (int i = 0; i < thread_count; ++i) {
	    FilterArgs* args = (FilterArgs *)alloc(sizeof(FilterArgs));
	    args->image = pgm;
	    args->filter = filter_matrix;
	    args->out = out_matrix; 
		args->k = i + 1;
		args->m = thread_count;
		args->optype = optype;

		call_args[i] = args;
	}

	int res;
	pthread_t *thread_tids = (pthread_t *)alloc(sizeof(pthread_t) * thread_count);

	unsigned long long int start = get_timestamp();

	for (int i = 0; i < thread_count; ++i) {
		res = pthread_create(&thread_tids[i], NULL, filter, (void *)call_args[i]); 

		if(res == -1) {
			fprintf(stderr, "Cannot create thread %d\n", i);
			exit(THREAD_CREATION);
		}
	}

	for (int i = 0; i < thread_count; ++i) {
		void *retbuf;
		res = pthread_join(thread_tids[i], &retbuf); 

		unsigned long long int thread_time = (unsigned long long int)retbuf;

		printf("tid: %lu - thread_time: %llu\n",
				thread_tids[i],
				thread_time); 

		if (res != 0) {
			fprintf(stderr, "Error joining thread %lu\n", thread_tids[i]);
		}
	}

	unsigned long long int end = get_timestamp();

	printf("Total time: %lld\n", end - start);

	save_out(out_matrix, out_filename);


	free_matrix(pgm);
	free_matrix(out_matrix);
	free_float_matrix(filter_matrix);

	for (int i = 0; i < thread_count; ++i) {
		free(call_args[i]);
	}
	free(call_args);
	free(thread_tids);
	return 0;
}


/**
 * Arguments:
 * 1. Thread count
 * 2. Type of work division - block / interleaved
 * 3. input image filename
 * 4. filter image filename
 * 5. output filename 
 **/
int check_args(int argc, char **argv) {
	if (argc != 6) {
		perror("Wrong argument number. Expected 5\n");
		return -1;
	}

	if(!is_num(argv[1])) {
		perror("1st arg must be numeric\n");
		return -1;
	}

	if (strcmp(argv[2], "block") != 0 &&
				strcmp(argv[2], "interleaved") != 0) {

		perror("2nd arg must be 'block' or 'interleaved'\n");
		return -1;
	}

	if (!file_exists(argv[3]) || !file_exists(argv[4])) {
		perror("input or filter file does not exits\n");
		return -1;
	}

	return 0;
}

void* filter(void *_args) {
	unsigned long long int start = get_timestamp(); 
	FilterArgs* args = (FilterArgs *)_args; 

	if (args->optype == BLOCK) {
		float block_width = (float)args->image->cols / (float)args->m;

		int start_idx = ceilf((args->k - 1) * block_width);	
		int end_idx = min(ceilf((args->k) * block_width), args->out->cols - 1);

		for (int i = start_idx; i <= end_idx; ++i) {
			filter_col(args->image,
					args->filter,
					args->out,
					i);
		}
	} else {
		for (int i = args->k - 1; i < args->image->cols; i += args->m) {
			filter_col(args->image,
					args->filter,
					args->out,
					i);
		}
	}

	unsigned long long int end = get_timestamp(); 

	return (void *)(end - start);
}

void filter_col(Matrix* image, MatrixFloat* filter, Matrix* out, int x) {
	float c = (float)filter->cols;

	for (int y = 0; y < image->rows; ++y) {
	    float pix = 0.0;

	    for (int i = 0; i < filter->rows; ++i) {
	    	for (int j = 0; j < filter->cols; ++j) {
				int idx_x = max(0, x - ceilf(c/2) + i - 1); 
				int idx_y = max(0, y - ceilf(c/2) + j - 1);
				
				if (idx_x >= image->cols || idx_y >= image->rows) {
					continue;
				}

				pix += (float)image->arr[idx_y][idx_x] * filter->arr[i][j]; 
	    	}
	    }

		out->arr[y][x] = roundf(pix);
	}
}

void save_out(Matrix * out, const char *outpath) {
	int max_val = 0;
	for (int i = 0; i < out->rows; ++i) {
		for (int j = 0; j < out->cols; ++j) {
			if (out->arr[i][j] > max_val) {
				max_val = out->arr[i][j];
			}
		}
	}

	FILE *f = fopen(outpath, "w+");
	if (f == NULL) {
		fprintf(stderr, "Cannot open file %s\n", outpath);
		exit(BAD_ARGS);
	}

	const char *format = "P2\n";
	fwrite(format, strlen(format), 1, f); 
	assert_file_ok(f, outpath);

	char buf[128];

	sprintf(buf, "%ld %ld\n", out->cols, out->rows);
	fwrite(buf, strlen(buf), 1, f);
	assert_file_ok(f, outpath);

	sprintf(buf, "%d\n", max_val);
	fwrite(buf, strlen(buf), 1, f);
	assert_file_ok(f, outpath);

	for (int i = 0; i < out->rows; ++i) {
		for (int j = 0; j < out->cols; ++j) {
    	    char buf[128];
    	    sprintf(buf, "%d ", out->arr[i][j]);
			fwrite(buf, strlen(buf), 1, f);
			assert_file_ok(f, outpath);
		}
		fwrite("\n", strlen("\n"), 1, f);
		assert_file_ok(f, outpath);
	}
	fwrite("\n", strlen("\n"), 1, f);
	assert_file_ok(f, outpath);
	fclose(f);
}

