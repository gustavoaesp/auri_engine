#pragma once
#include <array>
#include <math.h>
#include "math/matrix.hpp"

namespace eng
{

template <typename T, int ndimensions>
class Vector
{
public:
	Vector() : elem_{}
	{}

	template <typename ...Args>
	Vector(Args... args) :elem_{ args... }
	{}

	template <int ndiminferior, typename... Args>
	Vector(const Vector<T, ndiminferior>& v, Args... args):
		elem_(v.elem_, args...)
	{}

	Vector(const Vector<T, ndimensions>& v):
		elem_(v.elem_)
	{}

	void operator +=(const Vector<T, ndimensions>& v)
	{
		for (int i = 0; i < ndimensions; ++i)
		{
			elem_[i] += v.elem_[i];
		}
	}

	void operator -=(const Vector<T, ndimensions>& v)
	{
		for (int i = 0; i < ndimensions; ++i)
		{
			elem_[i] -= v.elem_[i];
		}
	}

	void operator *=(const Vector<T, ndimensions>& v)
	{
		for (int i = 0; i < ndimensions; ++i)
		{
			elem_[i] *= v.elem_[i];
		}
	}

	void operator *=(const T& a) {
		for (int i = 0; i < ndimensions; ++i) {
			elem_[i] *= a;
		}
	}

	void operator /=(const T& a)
	{
		for (int i = 0; i < ndimensions; ++i)
		{
			elem_[i] /= a;
		}
	}

	Vector<T, ndimensions> operator + (const Vector<T, ndimensions>& a) const
	{
		Vector<T, ndimensions> res(*this);
		res += a;
		return res;
	}

	Vector<T, ndimensions> operator - (const Vector<T, ndimensions>& a) const
	{
		Vector<T, ndimensions> res(*this);
		res -= a;
		return res;
	}

	Vector<T, ndimensions> operator * (const Vector<T, ndimensions>& a) const
	{
		Vector<T, ndimensions> res(*this);
		res *= a;
		return res;
	}
	Vector<T, ndimensions> operator * (const T &a) const
	{
		Vector<T, ndimensions> res(*this);
		res *= a;
		return res;
	}

	Vector<T, ndimensions> operator / (const T& a) const
	{
		Vector<T, ndimensions> res(*this);
		res /= a;
		return res;
	}
	
	Vector<T, ndimensions> operator *(const Matrix<T, ndimensions>& m) const
	{
		Vector<T, ndimensions> resp;
		for (int x = 0; x < ndimensions; x++)
		{
			for (int y = 0; y < ndimensions; y++)
			{
				resp.elem_[x] += m(y, x) * elem_[y];
			}
		}

		return resp;
	}

	T mag() const
	{
		T accum = 0;
		for (int i = 0; i < ndimensions; ++i)
		{
			accum += elem_[i] * elem_[i];
		}

		return sqrt(accum);
	}

	Vector<T, ndimensions> unit() const
	{
		Vector<T, ndimensions> res(*this);
		res /= mag();
		return res;
	}

	T* data()
	{
		return elem_.data();
	}

	const T* data() const
	{
		return elem_.data();
	}

	T& operator()(int i)
	{
		return elem_[i];
	}
	const T& operator()(int i) const
	{
		return elem_[i];
	}

    static float Dot(const Vector<T, ndimensions>& a, const Vector<T, ndimensions>& b)
    {     
        float dot = 0;
        for(int i = 0; i < ndimensions; ++i) {
            dot += a(i) * b(i);
        }

        return dot;
    }

	static Vector<T, ndimensions> Lerp(
		const Vector<T, ndimensions>& a,
		const Vector<T, ndimensions>& b,
		float t)
	{
		return a * (1.0f - t) + b * t;
	}

private:
	std::array<T, ndimensions> elem_;
};

using vec2f = Vector<float, 2>;
using vec3f = Vector<float, 3>;
using vec4f = Vector<float, 4>;

}
