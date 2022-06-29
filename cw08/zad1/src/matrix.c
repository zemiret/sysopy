#include <stdlib.h>
#include "../inc/utils.h"
#include "../inc/matrix.h"


Matrix * init_matrix(long rows, long cols) {
	Matrix *matrix = (Matrix *)alloc(sizeof(Matrix));
	matrix->rows = rows;
	matrix->cols = cols;

	matrix->arr = (short int **)alloc(sizeof(short int *) * rows);

	for (long i = 0; i < rows; ++i) {
		matrix->arr[i] = (short int *)alloc(sizeof(short int) * cols);
	}

	return matrix;
}

void free_matrix(Matrix *matrix) {
	long rows = matrix->rows;

	for (long i = 0; i < rows; ++i) {
		free(matrix->arr[i]);
	}
	free(matrix->arr);
	free(matrix);
}

MatrixFloat * init_float_matrix(long rows, long cols) {
	MatrixFloat *matrix = (MatrixFloat *)alloc(sizeof(MatrixFloat));
	matrix->rows = rows;
	matrix->cols = cols;

	matrix->arr = (float **)alloc(sizeof(float *) * rows);

	for (long i = 0; i < rows; ++i) {
		matrix->arr[i] = (float *)alloc(sizeof(float) * cols);
	}

	return matrix;
}

void free_float_matrix(MatrixFloat *matrix) {
	long rows = matrix->rows;

	for (long i = 0; i < rows; ++i) {
		free(matrix->arr[i]);
	}
	free(matrix->arr);
	free(matrix);
}
