#pragma once
#include "Shape.h"

namespace singleshape
{
#pragma pack(push, 1) //align to 1 byte
template<typename T, int N> struct arrayer
{
	T f[N];

	static const int num;
	static const int sz;
	static const void* vsz;
};
template<typename T, int N>  const int   arrayer<T, N>::num = N;
template<typename T, int N>  const int   arrayer<T, N>::sz  = sizeof(T[N]);
template<typename T, int N>  const void* arrayer<T, N>::vsz = reinterpret_cast<void*> (sizeof(T[N]));

template<typename T, int NV, int NC> struct vertexcolor
{
	using vt = arrayer <T, NV>;
	using ct = arrayer <T, NC>;
	vt v; ct c;
	static const void* vstart;
	static const void* cstart;
};

template<typename T, int NV, int NC> const void*   vertexcolor<T, NV, NC>::vstart = 0;
template<typename T, int NV, int NC> const void*   vertexcolor<T, NV, NC>::cstart = vertexcolor<T, NV, NC>::vt::vsz;

#pragma pack(pop)

class TrianglesPoli: public Shape
{
	using vcl = vertexcolor<float, 3, 4>;
	static const vcl obj[];

	unsigned int vao;

public:

	TrianglesPoli();
	TrianglesPoli(const char* vertexPath, const char* fragmentPath);

	void draw();
	void build();
};

}