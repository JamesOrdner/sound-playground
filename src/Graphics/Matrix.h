#pragma once

#include <initializer_list>
#include <cmath>
#include <cfloat>

namespace mat // Matrix
{
	/** Constants */

	const float pi = 3.14159265f;

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

		explicit constexpr Vector() : data{} {}

		explicit Vector(float* data) : data{} {
			for (int i = 0; i < 3; i++) this->data[i] = data[i];
		}

		constexpr explicit Vector(const Vector<T, 4>& v) : data{} {
			data[0] = v.data[0];
			data[1] = v.data[1];
			data[2] = v.data[2];
		}

		explicit constexpr Vector(T fill) : data{} {
			for (int i = 0; i < 3; i++) data[i] = fill;
		}

		explicit constexpr Vector(std::initializer_list<T> l) : data{} {
			const T* lp = l.begin();
			for (int i = 0; i < 3; i++) data[i] = lp[i];
		}

		constexpr T operator[](int i) const {
			return data[i];
		}
	};

	template<typename T>
	struct Vector<T, 4> {
		union {
			float data[4];
			struct { T x, y, z, w; };
			struct { T r, g, b, a; };
		};

		explicit constexpr Vector() : data{} {}

		explicit Vector(float* data) : data{} {
			for (int i = 0; i < 4; i++) this->data[i] = data[i];
		}

		constexpr explicit Vector(const Vector<T, 3>& v) : data{} {
			data[0] = v.data[0];
			data[1] = v.data[1];
			data[2] = v.data[2];
			data[3] = static_cast<T>(1);
		}

		explicit constexpr Vector(T fill) : data{} {
			for (int i = 0; i < 4; i++) data[i] = fill;
		}

		explicit constexpr Vector(std::initializer_list<T> l) : data{} {
			const T* lp = l.begin();
			for (int i = 0; i < 4; i++) data[i] = lp[i];
		}

		constexpr T operator[](int i) const {
			return data[i];
		}
	};
	
	/** Vector Typedefs */

	typedef Vector<float, 3> vec3;
	typedef Vector<float, 4> vec4;

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
	constexpr Vector<T, size> operator-(const Vector<T, size>& rhs) {
		Vector<T, size> v;
		for (int i = 0; i < size; i++) v.data[i] = -rhs.data[i];
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

	template <typename T, int size>
	constexpr Vector<T, size> operator*(const Vector<T, size>& lhs, const T& rhs) {
		Vector<T, size> v;
		for (int i = 0; i < size; i++) v.data[i] = lhs.data[i] * rhs;
		return v;
	}

	template <typename T, int size>
	constexpr Vector<T, size> operator*(const Vector<T, size>& lhs, const Vector<T, size>& rhs) {
		Vector<T, size> v;
		for (int i = 0; i < size; i++) v.data[i] = lhs.data[i] * rhs.data[i];
		return v;
	}

	/** Vector Functions */

	template <typename T, int size>
	constexpr T dot(const Vector<T, size>& a, const Vector<T, size>& b) {
		T d = 0;
		for (int i = 0; i < size; i++) d += a.data[i] * b.data[i];
		return d;
	}

	template <typename T, int size>
	T dist(const Vector<T, size>& a, const Vector<T, size>& b) {
		T d = 0;
		for (int i = 0; i < size; i++) d += std::powf(a.data[i] - b.data[i], 2.f);
		return std::sqrtf(d);
	}

	template <int size>
	constexpr Vector<float, size> normal(const Vector<float, size>& a) {
		float norm = 0;
		for (int i = 0; i < size; i++) norm += a.data[i] * a.data[i];
		if (norm < FLT_EPSILON) return Vector<float, size>();
		norm = std::sqrtf(norm); // sqrtf() prevents constexpr
		Vector<float, size> a_n;
		for (int i = 0; i < size; i++) a_n.data[i] = a.data[i] / norm;
		return a_n;
	}

	constexpr vec3 cross(const vec3& a, const vec3& b) {
		return vec3{
			a[1] * b[2] - a[2] * b[1],
			a[2] * b[0] - a[0] * b[2],
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

	/** Matrix Functions */

	template <typename T, int rows, int cols>
	constexpr Vector<T, cols> operator*(const Matrix<T, rows, cols>& lhs, const Vector<T, cols>& rhs) {
		Vector<T, rows> v;
		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < cols; c++) {
				v.data[r] += lhs.data[r][c] * rhs[c];
			}
		}
		return v;
	}

	// Transpose
	template <typename T, int rows, int cols>
	constexpr Matrix<T, cols, rows> t(const Matrix<T, rows, cols>& matrix) {
		Matrix<T, cols, rows> m;
		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < cols; c++) {
				m.data[c][r] = matrix.data[r][c];
			}
		}
		return m;
	}

	// Produces a view matrix with no translation -- camera remains at origin oriented up
	constexpr mat4 lookAt(const vec3& eye, const vec3& target) {
		vec3 up{ 0, 1, 0 };
		vec3 z = normal(eye - target);
		vec3 x = normal(cross(up, z));
		vec3 y = cross(z, x);

		return mat4{
			{ x[0], x[1], x[2], -dot(x, eye) },
			{ y[0], y[1], y[2], -dot(y, eye) },
			{ z[0], z[1], z[2], -dot(z, eye) },
			{    0,   0,    0,             1 }
		};
	}

	// Produce an orthographic projection matrix
	constexpr mat4 ortho(float l, float r, float b, float t, float f, float n) {
		return mat4{
			{ 2 / (r - l),           0,           0, -(r + l) / (r - l) },
			{           0, 2 / (t - b),           0, -(t + b) / (t - b) },
			{           0,           0, 2 / (f - n), -(f + n) / (f - n) },
			{           0,           0,           0,                  1 }
		};
	}

	// Produces a transform matrix
	mat4 transform(const vec3& loc, const vec3& rot, const vec3& scale);

	// Calculate matrix inverse
	mat4 inverse(const mat4& matrix);
}
