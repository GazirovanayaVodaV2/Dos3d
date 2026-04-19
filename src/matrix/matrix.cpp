#include <string.h>
#include "matrix.hpp"

matrix matrix::operator+(const matrix a) {
	matrix res;

	for (byte i = 0; i < 4; i++) {
		for (byte u = 0; u < 4; u++) {
			res.m_mat[i][u] = this->m_mat[i][u] + a.m_mat[i][u];
		}
	}

	return res;
}

matrix matrix::operator-(const matrix a) {
	matrix res;

	for (byte i = 0; i < 4; i++) {
		for (byte u = 0; u < 4; u++) {
			res.m_mat[i][u] = this->m_mat[i][u] - a.m_mat[i][u];
		}
	}

	return res;
}

matrix matrix::operator*(const matrix a) const {
	matrix res;
	for (int col = 0; col < 4; col++) {
		for (int row = 0; row < 4; row++) {
			res.m_mat[col][row] =
					this->m_mat[0][row] * a.m_mat[col][0] +
					this->m_mat[1][row] * a.m_mat[col][1] +
					this->m_mat[2][row] * a.m_mat[col][2] +
					this->m_mat[3][row] * a.m_mat[col][3];
		}
	}
	return res;
}


vec3 matrix::operator*(vec3 a) {
	vec3 res;
	res.x = a.x * this->m_mat[0][0] + a.y * this->m_mat[1][0] + a.z * this->m_mat[2][0] + this->m_mat[3][0];
	res.y = a.x * this->m_mat[0][1] + a.y * this->m_mat[1][1] + a.z * this->m_mat[2][1] + this->m_mat[3][1];
	res.z = a.x * this->m_mat[0][2] + a.y * this->m_mat[1][2] + a.z * this->m_mat[2][2] + this->m_mat[3][2];

	return res;
}

void matrix::translate(vec3 vec) {
	this->m_mat[3][0] += vec.x;
	this->m_mat[3][1] += vec.y;
	this->m_mat[3][2] += vec.z;
}


void matrix::identity() {
	static const float ident[4][4] = {
			{1, 0, 0, 0},
			{0, 1, 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 1},
	};

	memcpy(m_mat, ident, sizeof(m_mat));
}

vec3 matrix::project(vec3 vec) const {
	vec3 res;
	float w;
	res.x = vec.x * m_mat[0][0] + vec.y * m_mat[1][0] + vec.z * m_mat[2][0] + m_mat[3][0];
	res.y = vec.x * m_mat[0][1] + vec.y * m_mat[1][1] + vec.z * m_mat[2][1] + m_mat[3][1];
	res.z = vec.x * m_mat[0][2] + vec.y * m_mat[1][2] + vec.z * m_mat[2][2] + m_mat[3][2];

	w = vec.x * m_mat[0][3] + vec.y * m_mat[1][3] + vec.z * m_mat[2][3] + m_mat[3][3];

	if (w != 0.0f) {
		float invW = 1.0f / w;
		res.x *= invW;
		res.y *= invW;
		res.z *= invW;
	}

	return res;
}