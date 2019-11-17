#pragma once

#include <initializer_list>
#include <cmath>
#include <cfloat>

namespace Mat // Matrix
{
	/** Vector */

	template <typename T, int size>
	struct Vector
	{
		T data[size];

		explicit constexpr Vector() : data{} {}

		explicit constexpr Vector(T fill) : data{} {
			for (int i = 0; i < size; i++) data[i] = fill;
		}

		explicit constexpr Vector(std::initializer_list<T> l) : data{} {
			const T* lp = l.begin();
			for (int i = 0; i < size; i++) data[i] = lp[i];
		}

		constexpr T operator[](int i) const {
			return data[i];
		}
	};

	/** Vector Specializations */

	template<typename T>
	struct Vector<T, 3> {
		union {
			float data[3];
			struct { T x, y, z; };
			struct { T r, g, b; };
		};

		constexpr T operator[](int i) const {
			return data[i];
		}
	};
	
	/** Vector Typedefs */

	typedef Vector<float, 3> vec3;

	/** Vector Operators */

	template <typename T, int size>
	constexpr Vector<T, size> operator+(const Vector<T, size>& lhs, const T& rhs) {
		Vector<T, size> v;
		for (int i = 0; i < size; i++) v.data[i] = lhs.data[i] + rhs;
		return v;
	}

	template <typename T, int size>
	constexpr Vector<T, size> operator+(const Vector<T, size>& lhs, const Vector<T, size>& rhs) {
		Vector<T, size> v;
		for (int i = 0; i < size; i++) v.data[i] = lhs.data[i] + rhs.data[i];
		return v;
	}

	template <typename T, int size>
	constexpr Vector<T, size> operator-(const Vector<T, size>& lhs, const T& rhs) {
		Vector<T, size> v;
		for (int i = 0; i < size; i++) v.data[i] = lhs.data[i] - rhs;
		return v;
	}

	template <typename T, int size>
	constexpr Vector<T, size> operator-(const Vector<T, size>& lhs, const Vector<T, size>& rhs) {
		Vector<T, size> v;
		for (int i = 0; i < size; i++) v.data[i] = lhs.data[i] - rhs.data[i];
		return v;
	}

	/** Vector Functions */

	template <typename T, int size>
	constexpr T dot(const Vector<T, size>& a, const Vector<T, size>& b) {
		T d = 0;
		for (int i = 0; i < size; i++) d += a.data[i] * b.data[i];
		return d;
	}

	template <int size>
	constexpr Vector<float, size> normal(const Vector<float, size>& a) {
		float norm = 0;
		for (int i = 0; i < size; i++) norm += a.data[i] * a.data[i];
		if (norm < FLT_EPSILON) return Vector<float, size>();
		norm = sqrtf(norm);
		Vector<float, size> a_n;
		for (int i = 0; i < size; i++) a_n.data[i] = a.data[i] / norm;
		return a_n;
	}

	constexpr vec3 cross(const vec3& a, const vec3& b) {
		return vec3{
			a[1] * b[2] - a[2] * b[1],
			a[0] * b[2] - a[2] * b[0],
			a[0] * b[1] - a[1] * b[0]
		};
	}



	/** Matrix */

	template <typename T, int rows, int cols>
	struct Matrix {
		T data[rows][cols];

		explicit constexpr Matrix() : data{} {}

		explicit constexpr Matrix(T fill) : data{} {
			for (int r = 0; r < rows; r++) {
				for (int c = 0; c < cols; c++) {
					data[r][c] = fill;
				}
			}
		}

		explicit constexpr Matrix(std::initializer_list<std::initializer_list<T>> l) : data{} {
			for (int r = 0; r < rows; r++) {
				const std::initializer_list<T>* rp = l.begin();
				for (int c = 0; c < cols; c++) {
					const T* cp = rp[r].begin();
					data[r][c] = cp[c];
				}
			}
		}

		static constexpr Matrix Identity() {
			Matrix m;
			int max = rows < cols ? rows : cols;
			for (int i = 0; i < max; i++) m.data[i][i] = 1;
			return m;
		}

		const T& operator()(int r, int c) const {
			return data[r][c];
		}

		T& operator()(int r, int c) {
			return data[r][c];
		}

		constexpr Matrix<T, rows, cols> operator+(const T& other) const {
			Matrix<T, rows, cols> m;
			for (int r = 0; r < rows; r++) {
				for (int c = 0; c < cols; c++) {
					m.data[r][c] = data[r][c] + other;
				}
			}
			return m;
		}

		constexpr Matrix<T, rows, cols> operator+(const Matrix<T, rows, cols>& other) const {
			Matrix<T, rows, cols> m;
			for (int r = 0; r < rows; r++) {
				for (int c = 0; c < cols; c++) {
					m.data[r][c] = data[r][c] + other.data[r][c];
				}
			}
			return m;
		}

		constexpr Matrix<T, rows, cols> operator-(const T& other) const {
			Matrix<T, rows, cols> m;
			for (int r = 0; r < rows; r++) {
				for (int c = 0; c < cols; c++) {
					m.data[r][c] = data[r][c] - other;
				}
			}
			return m;
		}

		constexpr Matrix<T, rows, cols> operator-(const Matrix<T, rows, cols>& other) const {
			Matrix<T, rows, cols> m;
			for (int r = 0; r < rows; r++) {
				for (int c = 0; c < cols; c++) {
					m.data[r][c] = data[r][c] - other.data[r][c];
				}
			}
			return m;
		}

		template<int other_cols>
		constexpr Matrix<T, rows, other_cols> operator*(const Matrix<T, rows, other_cols>& other) const {
			Matrix<T, rows, other_cols> m;
			for (int m_r = 0; m_r < rows; m_r++) {
				for (int m_c = 0; m_c < other_cols; m_c++) {
					for (int i = 0; i < cols; i++) {
						m.data[m_r][m_c] += data[m_r][i] * other.data[i][m_c];
					}
				}
			}
			return m;
		}
	};

	typedef Matrix<float, 4, 4> mat4;

	template <typename T, int rows, int cols>
	constexpr Matrix<T, cols, rows> transpose(const Matrix<T, rows, cols>& matrix) {
		Matrix<T, cols, rows> m;
		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < cols; c++) {
				m.data[c][r] = matrix.data[r][c];
			}
		}
		return m;
	}

	mat4 lookAt(const vec3& eye, const vec3& target, const vec3& up) {
		// Orthonormal basis vectors
		vec3 z_basis = normal(eye - target);
		vec3 x_basis = normal(cross(up, z_basis));
		vec3 y_basis = cross(z_basis, x_basis);
		
		// Inverse orientation matrix
		mat4 orientation{
			{ x_basis.x, y_basis.x, z_basis.x, 0 },
			{ x_basis.y, y_basis.y, z_basis.y, 0 },
			{ x_basis.z, y_basis.z, z_basis.z, 0 },
			{         0,         0,         0, 1 }
		};

		// Inverse translation matrix
		mat4 translation{
			{      1,      0,      0,      0 },
			{      0,      1,      0,      0 },
			{      0,      0,      1,      0 },
			{ -eye.x, -eye.y, -eye.z,      1 }
		};

		return orientation * translation;
	}

	mat4 ortho(float l, float r, float b, float t, float f, float n) {
		return mat4{
			{ 2 / (r - l),           0,            0, -(r + l) / (r - l) },
			{           0, 2 / (t - b),            0, -(r + b) / (r - b) },
			{           0,           0, -2 / (f - n), -(f + n) / (f - n) },
			{           0,           0,            0,                  1 }
		};
	}
}
