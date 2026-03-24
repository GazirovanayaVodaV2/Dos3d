#ifndef MATRIX_H
#define MATRIX_H

#include "../utils/utils.h"

#include "../vec3/vec3.h"

typedef number matrix[16];

void madd(matrix *res, const matrix self, const matrix m_ref);
void msub(matrix *res, const matrix self, const matrix m_ref);

void mmult(matrix *res, const matrix self, const matrix m_ref);
vec3 mmult_vec(const matrix self, const vec3 vec);
void midentity(matrix *self);
void mtranslate(matrix *self, const vec3 vec);

#endif