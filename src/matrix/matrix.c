#include <string.h>
#include "matrix.h"

void madd(matrix *res, const matrix self, const matrix m_ref) {
	for (int i = 0; i < 16; i++) {
		(*res)[i] = self[i] + m_ref[i];
	}
}

void msub(matrix *res, const matrix self, const matrix m_ref) {
	for (int i = 0; i < 16; i++) {
		(*res)[i] = self[i] - m_ref[i];
	}
}

void mmult(matrix *res, const matrix self, const matrix m_ref) {
	float tmp[16];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			tmp[i * 4 + j] =
					self[i * 4 + 0] * m_ref[0 * 4 + j] +
					self[i * 4 + 1] * m_ref[1 * 4 + j] +
					self[i * 4 + 2] * m_ref[2 * 4 + j] +
					self[i * 4 + 3] * m_ref[3 * 4 + j];
		}
	}
	memcpy(*res, tmp, sizeof(float) * 16);
}

vec3 mmult_vec(const matrix m, vec3 v) {

	vec3 res;
	float w;
	res.x = v.x * (m)[0] + v.y * (m)[4] + v.z * (m)[8] + (m)[12];
	res.y = v.x * (m)[1] + v.y * (m)[5] + v.z * (m)[9] + (m)[13];
	res.z = v.x * (m)[2] + v.y * (m)[6] + v.z * (m)[10] + (m)[14];
	w = v.x * (m)[3] + v.y * (m)[7] + v.z * (m)[11] + (m)[15];

	if (w != 0.0f) {
		res.x /= w;
		res.y /= w;
		res.z /= w;
	}

	return res;
}

void midentity(matrix *m) {
	for (int i = 0; i < 16; i++) (*m)[i] = 0;
	(*m)[0] = (*m)[5] = (*m)[10] = (*m)[15] = 1.0f;
}

void mtranslate(matrix *m, vec3 vec) {
	midentity(m);
	(*m)[12] = vec.x;
	(*m)[13] = vec.y;
	(*m)[14] = vec.z;
}