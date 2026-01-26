#pragma once
#ifndef __MEMORY_HELPERS_H__
#define __MEMORY_HELPERS_H__
#include <iostream>
#include <stdarg.h>
#include <memory.h>
using namespace std;
template <class T, const int dimension = 1> class MultiArray
{
	int numelements;
	int sizes[dimension];
	int blocks[dimension];
	T* array;
	void setdimensions(int firstdimension, va_list  argptr)
	{
		sizes[0] = firstdimension;
		numelements = firstdimension;
		for( int i = 1 ; i < dimension; i++ )
		{
			sizes[i] = va_arg( argptr, int );
			numelements *= sizes[i];
		}
		for (int i = dimension - 1, blocksize = 1; i >= 0; blocksize *= sizes[i], i--)
			blocks[i] = blocksize;
		delete[] array;
		array = new T[numelements];
	}
	class manipulator
	{
		MultiArray<T, dimension>* elements;
		int sizer;
		T* p;
	public:
		manipulator(MultiArray<T, dimension>* _elements, int i): elements(_elements), p(_elements->array + _elements->blocks[0] * i), sizer(1)
		{
		}
		manipulator& operator[](int i)
		{
			if (sizer >= dimension) throw "operator[] too many calls";
			p += elements->blocks[sizer] * i;
			sizer ++;
			return *this;
		}
		operator T* ()
		{
			if (sizer >= dimension) throw "only arrays are allowed pointed dirrectly";
			return p;
		}
		T* set (T* x)
		{
			if (sizer >= dimension) throw "invalid index, operator ()(T*)";
			if (sizer <= 0) throw "invalid index, operator ()(T*)";
			memcpy(p, x, sizeof(T) * elements->blocks[sizer - 1]);
			return p;
		}
		T& operator () (T x)
		{
			if (sizer != dimension) throw "invalid index, operator ()(T)";
			(T&)*this = x;
			return *p;
		}
		T& operator () ()
		{
			if (sizer != dimension) throw "invalid index, operator ()()";
			return *p;
		}
		operator T& ()
		{
			if (sizer != dimension) throw "too many sizes, operator (T&)";
			return *p;
		}
	};

public:
	MultiArray():array(0), numelements(0) {memset(sizes, 0, sizeof(sizes));memset(blocks, 0, sizeof(blocks));}
	MultiArray(int firstdimension, ...) : array(0){
		va_list argptr;
		va_start(argptr, firstdimension);
		setdimensions(firstdimension, argptr);
		va_end(argptr);
	}
	void setdimensions(int firstdimension, ...)
	{
		va_list argptr;
		va_start(argptr, firstdimension);
		setdimensions(firstdimension, argptr);
		va_end(argptr);
	}
	manipulator operator[] (int i)
	{
		return manipulator (this, i);
	}
	~MultiArray()
	{
		delete[] array;
		array = 0;
	}
	void dump()
	{
		for (int i = 0; i < numelements; i++)
			std::cout<< "i:"<<i<<"="<< array[i]<< "; "<< std::endl;
	}
};
template <class T> class multiarray
{
	int* sizes;
	int dimension;
	T* array;
	void resetdimensions()
	{
		delete[] sizes;
		delete[] array;
		dimension = 0;
		sizes = 0;
		array = 0;
	}
	class manipulator
	{
		multiarray<T>* elements;
		int sizer;
		double* p;
		int getblock(int i)
		{
			int sz = i;
			if (sizer > elements->dimension + 1) throw;
			for (int i = sizer; i < elements->dimension; i++) sz *= elements->sizes[i];
			return sz;
		}
	public:
		manipulator(multiarray<T>* _elements, int i): elements(_elements), p(_elements->array), sizer(1)
		{
			p = elements->array + getblock(i);
			sizer++;
		}
		manipulator& operator[](int i)
		{
			p += getblock (i);
			sizer ++;
			return *this;
		}
		operator T* ()
		{
			if (sizer >= elements->dimension) throw "only arrays are allowed pointed dirrectly";
			return p;
		}
		T* set (T* x)
		{
			throw "Review this method before using";
			if (sizer >= elements->dimension) throw "invalid index, operator ()(T*)";
			if (sizer <= 0) throw "invalid index, operator ()(T*)";
			memcpy(p, x, sizeof(T) * elements->blocks[sizer - 1]);
			return p;
		}
		T& operator () (T x)
		{
			if (sizer > elements->dimension + 1) throw;
			(T&)*this = x;
			return *p;
		}
		operator T& ()
		{
			if (sizer > elements->dimension + 1) throw "too many sizes";
			sizer ++;
			return *p;
		}
	};
	void setdimensions(int _dimension, va_list  argptr)
	{
		dimension = _dimension;
		sizes = new int[_dimension];
		int numelements = 1;
		for( int i = 0 ; i < dimension; i++ )
		{
			sizes[i] = va_arg( argptr, int );
			numelements *= sizes[i];
			std::cout<< sizes[i]<< std::endl;
		}
		array = new T[numelements];
	}
public:
	multiarray():sizes(0), array(0), dimension(0){}
	multiarray(int _dimension, ...) {
		va_list argptr;
		va_start(argptr, _dimension);
		setdimensions(_dimension, argptr);
		va_end(argptr);
	}
	manipulator operator[] (int i)
	{
		return manipulator (this, i);
	}
	void setdimensions(int _dimension, ...) {
		va_list argptr;
		va_start(argptr, _dimension);
		setdimensions(_dimension, argptr);
		va_end(argptr);
	}

	void setdimensionss(int _dimension, int* _sizes)
	{
		resetdimensions();
		dimension =_dimension;
		sizes = new int[dimension];
		int numelements = 1;
		for (int i = 0; i < dimension; i++)
		{
			sizes[i] = _sizes[-i];
			numelements *= sizes[i];
		}
		array = new T[numelements];
	}
	void dump()
	{
		int numelements = 1;
		for (int i = 0; i < dimension; i++) numelements *= sizes[i];
		for (int i = 0; i < numelements; i++)
		{
			std::cout<< "i:"<<i<<"="<< array[i]<< "; "<< std::endl;
		}
	}
	void copymem(T* _array)
	{
		int numelements = 1;
		for (int i = 0; i < dimension; i++) numelements *= sizes[i];
		for (int i = 0; i < numelements; i++) array[i] = _array[i];
	}
	void attachmem(T* _array)
	{
		delete[] array;
		array = _array;
	}
	T* detachmem()
	{
		T* _array = array;
		array = 0;
		return _array;
	}
	~multiarray()
	{
		delete[] sizes;
		delete[] array;
	}
};
class points3
{
	int size1;
	int size2;
	double* c;
	class points3pointer
	{
		points3* points;
		int sizer;
		double* p;
	public:
		points3pointer(points3* _points, int i): points(_points), p(_points->c), sizer(2)
		{
			p = _points->c + (_points->size2 ) * i * 3;
		}
		points3pointer& operator[](int i)
		{
			sizer--;
			if (sizer == 1) p += i * 3;
			if (sizer == 0) p += i;
			return *this;
		}
		operator double* ()
		{
			if (sizer != 1) throw "sizer must be 1";
			sizer = 3;
			return p;
		}
		double& operator () (double x)
		{
			(double&)*this = x;
			return *p;
		}
		//		double& operator () (double x)
		//{
		//	(double&)*this = x;
		//	return *p;
		//}
		operator double& ()
		{
			if (sizer != 0) throw "sizer must be 0";
			sizer = 3;
			return *p;
		}
	};
public:
	points3(){c = 0;size1 = 0; size2 = 0;}

	void setSizes(int _size1, int _size2)
	{
		size1 = _size1;
		size2 = _size2;
		c = new double [size1 * size2 * 3];
	}
	points3pointer operator[](int i)
	{
		return points3pointer (this, i);
	}

	~points3(){delete[] c; c = 0;}
};
inline std::ostream& operator << (std::ostream& os, points3& pt)
{
	os<< (double&) pt;
	return os;
}

#endif