#ifndef MATRIX_H
#define MATRIX_H

#include "../utils/utils.h"

#include "../vec3/vec3.h"

typedef number matrix[16];

void madd(matrix *res, matrix *self, matrix *m_ref);
void msub(matrix *res, matrix *self, matrix *m_ref);

void mmult(matrix *res, matrix *self, matrix *m_ref);
vec3 mmult_vec(matrix *self, vec3 vec);
void midentity(matrix *self);
void mtranslate(matrix *self, vec3 vec);

#endif