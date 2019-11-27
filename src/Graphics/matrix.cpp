#include "Matrix.h"

using namespace mat;

vec3 mat::rotate(const vec3& vector, const vec3& rotation)
{
	mat4 rx{
		{ 1,                 0,                  0, 0 },
		{ 0, cosf(rotation[0]), -sinf(rotation[0]), 0 },
		{ 0, sinf(rotation[0]),  cosf(rotation[0]), 0 },
		{ 0,                 0,                  0, 1 }
	};

	mat4 ry{
		{  cosf(rotation[1]), 0, sinf(rotation[1]), 0 },
		{                  0, 1,                 0, 0 },
		{ -sinf(rotation[1]), 0, cosf(rotation[1]), 0 },
		{                  0, 0,                 0, 1 }
	};

	mat4 rz{
		{ cosf(rotation[2]), -sinf(rotation[2]), 0, 0 },
		{ sinf(rotation[2]),  cosf(rotation[2]), 0, 0 },
		{                 0,                  0, 1, 0 },
		{                 0,                  0, 0, 1 }
	};

	return vec3(rz * ry * rx * vec4(vector));
}

mat4 mat::transform(const vec3& loc, const vec3& rot, const vec3& scale) {
	mat4 s{
		{ scale[0],        0,        0, 0 },
		{        0, scale[1],        0, 0 },
		{        0,        0, scale[2], 0 },
		{        0,        0,        0, 1 }
	};

	mat4 rx{
		{ 1,            0,             0, 0 },
		{ 0, cosf(rot[0]), -sinf(rot[0]), 0 },
		{ 0, sinf(rot[0]),  cosf(rot[0]), 0 },
		{ 0,            0,             0, 1 }
	};

	mat4 ry{
		{  cosf(rot[1]), 0, sinf(rot[1]), 0 },
		{             0, 1,            0, 0 },
		{ -sinf(rot[1]), 0, cosf(rot[1]), 0 },
		{             0, 0,            0, 1 }
	};

	mat4 rz{
		{ cosf(rot[2]), -sinf(rot[2]), 0, 0 },
		{ sinf(rot[2]),  cosf(rot[2]), 0, 0 },
		{            0,             0, 1, 0 },
		{            0,             0, 0, 1 }
	};

	mat4 t{
		{ 1, 0, 0, loc[0] },
		{ 0, 1, 0, loc[1] },
		{ 0, 0, 1, loc[2] },
		{ 0, 0, 0,      1 }
	};

	return t * rz * ry * rx * s;
}

mat4 mat::inverse(const mat4& matrix)
{
	// TODO: Current implementation copied from glu, and not super efficient
	const float* m = &t(matrix).data[0][0];
	mat4 mtemp;
	float* inv = &mtemp.data[0][0];

	inv[0] = m[5] * m[10] * m[15] -
		m[5] * m[11] * m[14] -
		m[9] * m[6] * m[15] +
		m[9] * m[7] * m[14] +
		m[13] * m[6] * m[11] -
		m[13] * m[7] * m[10];

	inv[4] = -m[4] * m[10] * m[15] +
		m[4] * m[11] * m[14] +
		m[8] * m[6] * m[15] -
		m[8] * m[7] * m[14] -
		m[12] * m[6] * m[11] +
		m[12] * m[7] * m[10];

	inv[8] = m[4] * m[9] * m[15] -
		m[4] * m[11] * m[13] -
		m[8] * m[5] * m[15] +
		m[8] * m[7] * m[13] +
		m[12] * m[5] * m[11] -
		m[12] * m[7] * m[9];

	inv[12] = -m[4] * m[9] * m[14] +
		m[4] * m[10] * m[13] +
		m[8] * m[5] * m[14] -
		m[8] * m[6] * m[13] -
		m[12] * m[5] * m[10] +
		m[12] * m[6] * m[9];

	inv[1] = -m[1] * m[10] * m[15] +
		m[1] * m[11] * m[14] +
		m[9] * m[2] * m[15] -
		m[9] * m[3] * m[14] -
		m[13] * m[2] * m[11] +
		m[13] * m[3] * m[10];

	inv[5] = m[0] * m[10] * m[15] -
		m[0] * m[11] * m[14] -
		m[8] * m[2] * m[15] +
		m[8] * m[3] * m[14] +
		m[12] * m[2] * m[11] -
		m[12] * m[3] * m[10];

	inv[9] = -m[0] * m[9] * m[15] +
		m[0] * m[11] * m[13] +
		m[8] * m[1] * m[15] -
		m[8] * m[3] * m[13] -
		m[12] * m[1] * m[11] +
		m[12] * m[3] * m[9];

	inv[13] = m[0] * m[9] * m[14] -
		m[0] * m[10] * m[13] -
		m[8] * m[1] * m[14] +
		m[8] * m[2] * m[13] +
		m[12] * m[1] * m[10] -
		m[12] * m[2] * m[9];

	inv[2] = m[1] * m[6] * m[15] -
		m[1] * m[7] * m[14] -
		m[5] * m[2] * m[15] +
		m[5] * m[3] * m[14] +
		m[13] * m[2] * m[7] -
		m[13] * m[3] * m[6];

	inv[6] = -m[0] * m[6] * m[15] +
		m[0] * m[7] * m[14] +
		m[4] * m[2] * m[15] -
		m[4] * m[3] * m[14] -
		m[12] * m[2] * m[7] +
		m[12] * m[3] * m[6];

	inv[10] = m[0] * m[5] * m[15] -
		m[0] * m[7] * m[13] -
		m[4] * m[1] * m[15] +
		m[4] * m[3] * m[13] +
		m[12] * m[1] * m[7] -
		m[12] * m[3] * m[5];

	inv[14] = -m[0] * m[5] * m[14] +
		m[0] * m[6] * m[13] +
		m[4] * m[1] * m[14] -
		m[4] * m[2] * m[13] -
		m[12] * m[1] * m[6] +
		m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
		m[1] * m[7] * m[10] +
		m[5] * m[2] * m[11] -
		m[5] * m[3] * m[10] -
		m[9] * m[2] * m[7] +
		m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
		m[0] * m[7] * m[10] -
		m[4] * m[2] * m[11] +
		m[4] * m[3] * m[10] +
		m[8] * m[2] * m[7] -
		m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
		m[0] * m[7] * m[9] +
		m[4] * m[1] * m[11] -
		m[4] * m[3] * m[9] -
		m[8] * m[1] * m[7] +
		m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
		m[0] * m[6] * m[9] -
		m[4] * m[1] * m[10] +
		m[4] * m[2] * m[9] +
		m[8] * m[1] * m[6] -
		m[8] * m[2] * m[5];

	float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0) return mat4(); // no inverse

	det = 1.f / det;

	mat4 invOut;
	for (int i = 0; i < 16; i++)
		invOut.data[i / 4][i % 4] = inv[i] * det;

	return t(invOut);
}
