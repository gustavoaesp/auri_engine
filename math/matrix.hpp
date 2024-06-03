#pragma once
#include <array>

namespace eng
{

template <typename T, int dim>
class Matrix
{
public:
	Matrix() : elem_{}
	{}

	template<typename ...Args>
	Matrix(Args... args) : elem_{ args... }
	{}

	Matrix(const Matrix<T, dim>& m):
		elem_(m.elem_)
	{}

	void operator += (const Matrix<T, dim>& m)
	{
		for (int i = 0; i < dim*dim; ++i) {
			elem_[i] += m.elem_[i];
		}
	}

	void operator -= (const Matrix<T, dim>& m)
	{
		for (int i = 0; i < dim*dim; ++i) {
			elem_[i] -= m.elem_[i];
		}
	}

	void operator *= (const Matrix<T, dim>& m)
	{
		Matrix<T, dim> temp;
		for (int y = 0; y < dim; y++)
		{
			for (int x = 0; x < dim; x++)
			{
				for (int k = 0; k < dim; k++)
				{
					temp.elem_[array_access(y, x)] +=
						elem_[array_access(y, k)] * m.elem_[array_access(k, x)];
				}
			}
		}
		*this = temp;
	}


	Matrix<T, dim> operator + (const Matrix<T, dim>& m)
	{
		Matrix<T, dim> res(*this);
		res += m;
		return res;
	}

	Matrix<T, dim> operator - (const Matrix<T, dim>& m)
	{
		Matrix<T, dim> res(*this);
		res -= m;
		return res;
	}

	Matrix<T, dim> operator * (const Matrix<T, dim>& m)
	{
		Matrix<T, dim> res(*this);
		res *= m;
		return res;
	}

    T& operator()(int iy, int ix) {
		return elem_[array_access(iy, ix)];
	}

	const T& operator()(int iy, int ix) const
	{
		return elem_[array_access(iy, ix)];
	}

    const T* data() const
    {
        return elem_.data();
    }

	T* data()
	{
		return elem_.data();
	}

private:
	static inline int array_access(int y, int x)
	{
		return y * dim + x;
	}
	std::array<T, dim*dim> elem_;
};

using mtx2f = Matrix<float, 2>;
using mtx3f = Matrix<float, 3>;
using mtx4f = Matrix<float, 4>;

static mtx4f inverse(const mtx4f& m)
{
    mtx4f inv;
	float det;

    inv.data()[0] = m.data()[5]  * m.data()[10] * m.data()[15] - 
             m.data()[5]  * m.data()[11] * m.data()[14] - 
             m.data()[9]  * m.data()[6]  * m.data()[15] + 
             m.data()[9]  * m.data()[7]  * m.data()[14] +
             m.data()[13] * m.data()[6]  * m.data()[11] - 
             m.data()[13] * m.data()[7]  * m.data()[10];

    inv.data()[4] = -m.data()[4]  * m.data()[10] * m.data()[15] + 
              m.data()[4]  * m.data()[11] * m.data()[14] + 
              m.data()[8]  * m.data()[6]  * m.data()[15] - 
              m.data()[8]  * m.data()[7]  * m.data()[14] - 
              m.data()[12] * m.data()[6]  * m.data()[11] + 
              m.data()[12] * m.data()[7]  * m.data()[10];

    inv.data()[8] = m.data()[4]  * m.data()[9] * m.data()[15] - 
             m.data()[4]  * m.data()[11] * m.data()[13] - 
             m.data()[8]  * m.data()[5] * m.data()[15] + 
             m.data()[8]  * m.data()[7] * m.data()[13] + 
             m.data()[12] * m.data()[5] * m.data()[11] - 
             m.data()[12] * m.data()[7] * m.data()[9];

    inv.data()[12] = -m.data()[4]  * m.data()[9] * m.data()[14] + 
               m.data()[4]  * m.data()[10] * m.data()[13] +
               m.data()[8]  * m.data()[5] * m.data()[14] - 
               m.data()[8]  * m.data()[6] * m.data()[13] - 
               m.data()[12] * m.data()[5] * m.data()[10] + 
               m.data()[12] * m.data()[6] * m.data()[9];

    inv.data()[1] = -m.data()[1]  * m.data()[10] * m.data()[15] + 
              m.data()[1]  * m.data()[11] * m.data()[14] + 
              m.data()[9]  * m.data()[2] * m.data()[15] - 
              m.data()[9]  * m.data()[3] * m.data()[14] - 
              m.data()[13] * m.data()[2] * m.data()[11] + 
              m.data()[13] * m.data()[3] * m.data()[10];

    inv.data()[5] = m.data()[0]  * m.data()[10] * m.data()[15] - 
             m.data()[0]  * m.data()[11] * m.data()[14] - 
             m.data()[8]  * m.data()[2] * m.data()[15] + 
             m.data()[8]  * m.data()[3] * m.data()[14] + 
             m.data()[12] * m.data()[2] * m.data()[11] - 
             m.data()[12] * m.data()[3] * m.data()[10];

    inv.data()[9] = -m.data()[0]  * m.data()[9] * m.data()[15] + 
              m.data()[0]  * m.data()[11] * m.data()[13] + 
              m.data()[8]  * m.data()[1] * m.data()[15] - 
              m.data()[8]  * m.data()[3] * m.data()[13] - 
              m.data()[12] * m.data()[1] * m.data()[11] + 
              m.data()[12] * m.data()[3] * m.data()[9];

    inv.data()[13] = m.data()[0]  * m.data()[9] * m.data()[14] - 
              m.data()[0]  * m.data()[10] * m.data()[13] - 
              m.data()[8]  * m.data()[1] * m.data()[14] + 
              m.data()[8]  * m.data()[2] * m.data()[13] + 
              m.data()[12] * m.data()[1] * m.data()[10] - 
              m.data()[12] * m.data()[2] * m.data()[9];

    inv.data()[2] = m.data()[1]  * m.data()[6] * m.data()[15] - 
             m.data()[1]  * m.data()[7] * m.data()[14] - 
             m.data()[5]  * m.data()[2] * m.data()[15] + 
             m.data()[5]  * m.data()[3] * m.data()[14] + 
             m.data()[13] * m.data()[2] * m.data()[7] - 
             m.data()[13] * m.data()[3] * m.data()[6];

    inv.data()[6] = -m.data()[0]  * m.data()[6] * m.data()[15] + 
              m.data()[0]  * m.data()[7] * m.data()[14] + 
              m.data()[4]  * m.data()[2] * m.data()[15] - 
              m.data()[4]  * m.data()[3] * m.data()[14] - 
              m.data()[12] * m.data()[2] * m.data()[7] + 
              m.data()[12] * m.data()[3] * m.data()[6];

    inv.data()[10] = m.data()[0]  * m.data()[5] * m.data()[15] - 
              m.data()[0]  * m.data()[7] * m.data()[13] - 
              m.data()[4]  * m.data()[1] * m.data()[15] + 
              m.data()[4]  * m.data()[3] * m.data()[13] + 
              m.data()[12] * m.data()[1] * m.data()[7] - 
              m.data()[12] * m.data()[3] * m.data()[5];

    inv.data()[14] = -m.data()[0]  * m.data()[5] * m.data()[14] + 
               m.data()[0]  * m.data()[6] * m.data()[13] + 
               m.data()[4]  * m.data()[1] * m.data()[14] - 
               m.data()[4]  * m.data()[2] * m.data()[13] - 
               m.data()[12] * m.data()[1] * m.data()[6] + 
               m.data()[12] * m.data()[2] * m.data()[5];

    inv.data()[3] = -m.data()[1] * m.data()[6] * m.data()[11] + 
              m.data()[1] * m.data()[7] * m.data()[10] + 
              m.data()[5] * m.data()[2] * m.data()[11] - 
              m.data()[5] * m.data()[3] * m.data()[10] - 
              m.data()[9] * m.data()[2] * m.data()[7] + 
              m.data()[9] * m.data()[3] * m.data()[6];

    inv.data()[7] = m.data()[0] * m.data()[6] * m.data()[11] - 
             m.data()[0] * m.data()[7] * m.data()[10] - 
             m.data()[4] * m.data()[2] * m.data()[11] + 
             m.data()[4] * m.data()[3] * m.data()[10] + 
             m.data()[8] * m.data()[2] * m.data()[7] - 
             m.data()[8] * m.data()[3] * m.data()[6];

    inv.data()[11] = -m.data()[0] * m.data()[5] * m.data()[11] + 
               m.data()[0] * m.data()[7] * m.data()[9] + 
               m.data()[4] * m.data()[1] * m.data()[11] - 
               m.data()[4] * m.data()[3] * m.data()[9] - 
               m.data()[8] * m.data()[1] * m.data()[7] + 
               m.data()[8] * m.data()[3] * m.data()[5];

    inv.data()[15] = m.data()[0] * m.data()[5] * m.data()[10] - 
              m.data()[0] * m.data()[6] * m.data()[9] - 
              m.data()[4] * m.data()[1] * m.data()[10] + 
              m.data()[4] * m.data()[2] * m.data()[9] + 
              m.data()[8] * m.data()[1] * m.data()[6] - 
              m.data()[8] * m.data()[2] * m.data()[5];

    det = m.data()[0] * inv.data()[0] + m.data()[1] * inv.data()[4] + m.data()[2] * inv.data()[8] + m.data()[3] * inv.data()[12];

    det = 1.0 / det;

    for (int i = 0; i < 16; i++) {
        inv.data()[i] *= det;
	}

	return inv;
}

}
