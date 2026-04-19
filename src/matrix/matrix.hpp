#ifndef MATRIX_H
#define MATRIX_H

#include "../utils/utils.hpp"

#include "../vec3/vec3.hpp"

struct matrix {
	float m_mat[4][4] = {};

	matrix() {
		identity();
	}

	matrix(const vec3 &v) {
		identity();
		m_mat[3][0] = v.x;
		m_mat[3][1] = v.y;
		m_mat[3][2] = v.z;
	}

	matrix(const float *matrix) {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				m_mat[i][j] = matrix[i * 4 + j];
			}
		}
	}

	matrix operator+(const matrix a);
	matrix operator-(const matrix a);
	matrix operator*(const matrix a) const;
	vec3 operator*(vec3 a);

	vec3 project(vec3 vec) const;

	void translate(vec3 vec);

	void identity();
};

#endif