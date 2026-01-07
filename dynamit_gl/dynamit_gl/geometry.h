#pragma once
#include <array>

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
