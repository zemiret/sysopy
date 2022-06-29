#ifndef PGM_H
#define PGM_H

#include "matrix.h"

Matrix * read_pgm(const char *filename);
MatrixFloat *read_filter(const char *filename);

#endif
