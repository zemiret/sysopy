#ifndef MATRIX_H
#define MATRIX_H

typedef struct {
	long rows;
	long cols;
	short int **arr; 
} Matrix;

typedef struct {
	long rows;
	long cols;
	float **arr; 
} MatrixFloat;


Matrix * init_matrix(long rows, long cols);
void free_matrix(Matrix *matrix);

MatrixFloat * init_float_matrix(long rows, long cols);
void free_float_matrix(MatrixFloat *matrix);

#endif
