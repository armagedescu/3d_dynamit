#pragma once
#include <array>
#include <cmath>
#include <vector>

#if _MSVC_LANG  >= 201703L

template<class T, class ... T0> void resize(const T& r, T0& ... ts) { ((ts *= r), ...); };
template<class T, class ... T0> void shrink(const T& r, T0& ... ts) { ((ts /= r), ...); };

template<class T, class ... T1> T squaret(T1 ... t) { return ((t * t) + ...); }
template<class T, class ... T1> T hypn(T1 ... ts) { return std::sqrt(squaret<T>(ts ...)); }

#else
template<class T> void shrink(const T& r, T& ts) { ts /= r; }
template<class T, class ... T0> void shrink(const T& r, T& t0, T0& ... ts) { t0 /= r; shrink(r, ts ...); };
template<class T> void resize(const T& r, T& ts) { ts *= r; }
template<class T, class ... T0> void resize(const T& r, T& t0, T0& ... ts) { t0 *= r; resize(r, ts ...); };

template<class T> T squaret(T t) { return t * t; }
template<class T, class ... T2> T squaret(T t, T2 ... ts) { return t * t + squaret(ts ...); }

template<class T, class ... T2> T hypn(T t, T2 ... ts) { return std::sqrt(squaret(t, ts ...)); }

#endif
#pragma once

// Left-handed cross product for 3 points
inline std::array<float, 3> cross3pl(const std::array<float, 3>& p1,
	const std::array<float, 3>& p2,
	const std::array<float, 3>& p3)
{
	// v1 = p2 - p1
	float v1[3] = { p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2] };
	// v2 = p3 - p1
	float v2[3] = { p3[0] - p1[0], p3[1] - p1[1], p3[2] - p1[2] };

	// Left-handed cross product
	return {
		v1[2] * v2[1] - v1[1] * v2[2],
		v1[0] * v2[2] - v1[2] * v2[0],
		v1[1] * v2[0] - v1[0] * v2[1]
	};
}
//template<class T, auto deleter> struct cleaner
//{
//	T resource;
//	cleaner(const cleaner&) = delete;
//	cleaner(cleaner&&) = delete;
//	void operator=(const cleaner&) = delete;
//	cleaner* operator &() = delete;
//	~cleaner() { deleter(resource); }
//};

template<class T> void offset3(T& x, T& y, T& z, T dx, T dy, T dz) { x += dx; y += dy; z += dz; }
template<class T> void resize3(T& x, T& y, T& z, T rx, T ry, T rz) { x *= rx; y *= ry; z *= rz; }

//calculate normals
template<class T> void norm3(T& x, T& y, T& z, T x1, T y1, T z1, T x2, T y2, T z2)
{
	// |  i  j  k |
	// | x1 y1 z1 |
	// | x2 y2 z2 |
	x = y1 * z2 - z1 * y2;
	y = z1 * x2 - x1 * z2;
	z = x1 * y2 - y1 * x2;
}
//calculate normals normalzied
template<class T> void norm3nz(T& x, T& y, T& z, const T& x1, const T& y1, const T& z1, const T& x2, const T& y2, const T& z2, const T& rs = 1)
{
	norm3(x, y, z, x1, y1, z1, x2, y2, z2);
	T h = hypn<T> (x, y, z);
	shrink(h, x, y, z); // , h);
	resize<T>(rs, x, y, z); // , h);
};

template<int n, class T, class TO> TO& coutn(TO& os, T* t, const char* start = "{", const char* separ = "; ", const char* end = "}")
{
	os << start;
	for (int i = 0; i < n; i++)
		os << *(t + i) << separ;
	os << end;
	return os;
}


namespace dynamit::geo
{

template<typename T = float> using mat4 = std::array<T, 16>;
template<typename T = float> using mat3 = std::array<T, 9>;
template<typename T = float> using mat2 = std::array<T, 4>;

//TODO: scaleMatrix does not follow the naming convention

// Convention
// identity_* creates an identity matrix in the target/return value
// rotate_* rotates the target matrix by the given angle
// rotation_* creates a new rotation by the given angle matrix in the target/return value
// multiply_* multiplies the target matrix by the given matrix
template<typename T = float> constexpr mat4<T> identity_mat4()
{
    return {
        T{1}, T{0}, T{0}, T{0},
        T{0}, T{1}, T{0}, T{0},
        T{0}, T{0}, T{1}, T{0},
        T{0}, T{0}, T{0}, T{1}
    };
}
template<typename T = float> constexpr mat4<T> null_mat4()
{
    return {
        T{0}, T{0}, T{0}, T{0},
        T{0}, T{0}, T{0}, T{0},
        T{0}, T{0}, T{0}, T{0},
        T{0}, T{0}, T{0}, T{0}
    };
}
template<typename T = float> constexpr mat3<T> identity_mat3()
{
    return {
        T{1}, T{0}, T{0},
        T{0}, T{1}, T{0},
        T{0}, T{0}, T{1}
    };
}
template<typename T = float> constexpr mat3<T> null_mat3()
{
    return {
        T{0}, T{0}, T{0},
        T{0}, T{0}, T{0},
        T{0}, T{0}, T{0}
    };
}
template<typename T = float> constexpr mat2<T> identity_mat2()
{
    return {
        T{1}, T{0},
        T{0}, T{1}
    };
}
template<typename T = float> constexpr mat2<T> null_mat2()
{
    return {
        T{0}, T{0},
        T{0}, T{0}
    };
}

template<typename T = float> void multiply_mat3(const mat3<T>& a, mat3<T>& ioResult)
{
    mat3<T> temp = ioResult;  // Copy first
    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            ioResult[row + col * 3] =
                a[row + 0 * 3] * temp[0 + col * 3] +
                a[row + 1 * 3] * temp[1 + col * 3] +
                a[row + 2 * 3] * temp[2 + col * 3];
        }
    }
}

template<typename T = float> void multiply_mat4(const mat4<T>& a, mat4<T>& ioResult)
{
    mat4<T> temp = ioResult;  // Copy first
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            ioResult[row + col * 4] =
                a[row + 0 * 4] * temp[0 + col * 4] +
                a[row + 1 * 4] * temp[1 + col * 4] +
                a[row + 2 * 4] * temp[2 + col * 4] +
                a[row + 3 * 4] * temp[3 + col * 4];
        }
    }
}

template<typename T = float, typename T1> mat4<T> translation_mat4(T1 tx, T1 ty, T1 tz)
{
    return {
        T{1}, T{0}, T{0}, T{0},
        T{0}, T{1}, T{0}, T{0},
        T{0}, T{0}, T{1}, T{0},
        T{tx}, T{ty}, T{tz}, T{1}
    };
}

// Helper to create a scale matrix (column-major)
template<typename T = float, typename T1> mat4<T> scaleMatrix(T1 sx, T1 sy, T1 sz)
{
    return {
        T{sx},   T{0}, T{0}, T{0},
        T{0}, T{sy},   T{0}, T{0},
        T{0}, T{0}, T{sz},   T{0},
        T{0}, T{0}, T{0}, T{1}
    };
}

template<typename T = float> mat4<T> rotation_x_mat4(float angle)
{
    T c = std::cos(angle), s = std::sin(angle);
    return {
        T{1}, T{0}, T{0}, T{0},
        T{0}, c,   -s,    T{0},
        T{0}, s,   c,    T{0},
        T{0}, T{0}, T{0}, T{1}
    };
}
template<typename T = float> void rotation_x_mat4(float angle, mat4<T>& matrix)
{
    T c = std::cos(angle), s = std::sin(angle);
    matrix = {
        T{1}, T{0}, T{0}, T{0},
        T{0}, c,   -s,    T{0},
        T{0}, s,   c,    T{0},
        T{0}, T{0}, T{0}, T{1}
    };
}
template<typename T = float> void rotate_x_mat4(T angle, mat4<T>& ioResult)
{
    multiply_mat4(rotation_x_mat4(angle), ioResult);
}

template<typename T = float> mat4<T> rotation_y_mat4(float angle)
{
    T c = std::cos(angle), s = std::sin(angle);
    return {
        c,    T{0}, s,    T{0},
        T{0}, T{1}, T{0}, T{0},
        -s,   T{0}, c,    T{0},
        T{0}, T{0}, T{0}, T{1}
    };
}
template<typename T = float> void rotation_y_mat4(float angle, mat4<T>& matrix)
{
    T c = std::cos(angle), s = std::sin(angle);
    matrix = {
        c,    T{0}, s,    T{0},
        T{0}, T{1}, T{0}, T{0},
        -s,   T{0}, c,    T{0},
        T{0}, T{0}, T{0}, T{1}
    };
}
template<typename T = float> void rotate_y_mat4(T angle, mat4<T>& ioResult)
{
    multiply_mat4(rotation_y_mat4(angle), ioResult);
}
//TODO: clarify s/-s for Y
// Helper to create a rotation matrix around Z axis (column-major)
template<typename T = float> mat4<T> rotation_z_mat4(float angle)
{
    T c = std::cos(angle), s = std::sin(angle);
    return {
        c,   -s,    T{0}, T{0},
        s,   c,    T{0}, T{0},
        T{0}, T{0}, T{1}, T{0},
        T{0}, T{0}, T{0}, T{1}
    };
}
template<typename T = float> void rotation_z_mat4(float angle, mat4<T>& matrix)
{
    T c = std::cos(angle), s = std::sin(angle);
    matrix = {
        c,   -s,    T{0}, T{0},
        s,   c,    T{0}, T{0},
        T{0}, T{0}, T{1}, T{0},
        T{0}, T{0}, T{0}, T{1}
    };
}
template<typename T = float> void rotate_z_mat4(T angle, mat4<T>& ioResult)
{
    multiply_mat4(rotation_z_mat4(angle), ioResult);
}

inline void transformPosition(const mat4<float>& m, float& x, float& y, float& z)
{
    float w = 1.0f;
    float nx = m[0] * x + m[4] * y + m[8] * z + m[12] * w;
    float ny = m[1] * x + m[5] * y + m[9] * z + m[13] * w;
    float nz = m[2] * x + m[6] * y + m[10] * z + m[14] * w;
    x = nx;
    y = ny;
    z = nz;
}

//// Transform a normal (applies only rotation/scale, no translation)

//// Transform a normal (applies only rotation/scale, no translation)
//inline void transformNormal(const mat4<float>& m, float& nx, float& ny, float& nz)
//{
//    float tnx = m[0] * nx + m[4] * ny + m[8] * nz;
//    float tny = m[1] * nx + m[5] * ny + m[9] * nz;
//    float tnz = m[2] * nx + m[6] * ny + m[10] * nz;
//
//    float len = std::sqrt(tnx * tnx + tny * tny + tnz * tnz);
//    if (len > 0.0001f)
//    {
//        nx = tnx / len;
//        ny = tny / len;
//        nz = tnz / len;
//    }
//    else
//    {
//        nx = tnx;
//        ny = tny;
//        nz = tnz;
//    }
//}
//// Transform a normal correctly for non-uniform scaling
inline void transformNormal(const mat4<float>& m, float& nx, float& ny, float& nz)
{
    // For correct normal transformation under non-uniform scale,
    // we need to apply the inverse transpose of the 3x3 upper-left.
    // For scale matrix S and rotation R where M = R*S or S*R,
    // the inverse transpose scales by 1/s^2 per axis then applies rotation.

    float col0_sq = m[0] * m[0] + m[1] * m[1] + m[2] * m[2];
    float col1_sq = m[4] * m[4] + m[5] * m[5] + m[6] * m[6];
    float col2_sq = m[8] * m[8] + m[9] * m[9] + m[10] * m[10];

    // Normalize the matrix columns to get pure rotation, then apply
    float r00 = m[0] / col0_sq, r10 = m[1] / col0_sq, r20 = m[2] / col0_sq;
    float r01 = m[4] / col1_sq, r11 = m[5] / col1_sq, r21 = m[6] / col1_sq;
    float r02 = m[8] / col2_sq, r12 = m[9] / col2_sq, r22 = m[10] / col2_sq;

    float tnx = r00 * nx + r01 * ny + r02 * nz;
    float tny = r10 * nx + r11 * ny + r12 * nz;
    float tnz = r20 * nx + r21 * ny + r22 * nz;

    float len = std::sqrt(tnx * tnx + tny * tny + tnz * tnz);
    if (len > 0.0001f)
    {
        nx = tnx / len;
        ny = tny / len;
        nz = tnz / len;
    }
    else
    {
        nx = tnx;
        ny = tny;
        nz = tnz;
    }
}
//// Apply single transformation to a range of vertices and normals
// Apply single transformation to a range of vertices and normals
inline void applyTransformToRange(
    const mat4<float>& m,
    std::vector<float>& verts,
    std::vector<float>& norms,
    size_t startVertex)
{
    size_t startIdx = startVertex * 3;

    for (size_t i = startIdx; i < verts.size(); i += 3)
    {
        transformPosition(m, verts[i], verts[i + 1], verts[i + 2]);
    }
    
    for (size_t i = startIdx; i < norms.size(); i += 3)
    {
        transformNormal(m, norms[i], norms[i + 1], norms[i + 2]);
    }
}

// Variadic: apply multiple transformations in sequence
template<typename... Transforms> inline void applyTransformsToRange(
    std::vector<float>& verts,
    std::vector<float>& norms,
    size_t startVertex,
    const Transforms&... transforms)
{
    (applyTransformToRange(transforms, verts, norms, startVertex), ...);
}
}
