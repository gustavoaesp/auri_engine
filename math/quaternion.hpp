#pragma once

#include <math.h>
#include "math/matrix.hpp"
#include "math/vector.hpp"

namespace eng
{

class Quaternion
{
public:
    Quaternion(float x, float y, float z, float q0):
        q0(q0), x(x), y(y), z(z)
    {}

    mtx4f toMatrix() {

        /*return mtx4f(
            1.0f - 2*y*y - 2*z*z, 2*x*y - 2*q0*z, 2*x*z + 2*q0*y, 0.0f,
            2*x*y + 2*q0*z, 1 - 2*x*x - 2*z*z, 2*y*z - 2*q0*x, 0.0f,
            2*x*y - 2*q0*y, 2*y*z + 2*q0*y, 1 - 2*x*x - 2*y*y, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );*/

        return mtx4f(
            1 - 2*y*y - 2*z*z,          2*x*y + 2*q0*z,        2*x*z - 2*q0*y,  0.0f,
               2*x*y - 2*q0*z,       1 - 2*x*x - 2*z*z,        2*y*z + 2*q0*x,  0.0f,
               2*x*z + 2*q0*y,          2*y*z - 2*q0*x,     1 - 2*x*x - 2*y*y,  0.0f,
                         0.0f,                    0.0f,                  0.0f,  1.0f
        );

/*
        return mtx4f(
            1 - 2*y*y - 2*z*z,          2*x*y - 2*q0*z,        -2*x*z - 2*q0*y,  0.0f,
               2*x*y + 2*q0*z,       1 - 2*x*x - 2*z*z,        -2*y*z + 2*q0*x,  0.0f,
               -2*x*z + 2*q0*y,          -2*y*z - 2*q0*x,     1 - 2*x*x - 2*y*y,  0.0f,
                         0.0f,                    0.0f,                  0.0f,  1.0f
        );*/

    }

    void setAxis(const vec3f& a)
    {
        axis = a.unit();
        x = axis(0) * sin(rotation);
        y = axis(1) * sin(rotation);
        z = axis(2) * sin(rotation);
    }

    void setAxisRotation(const vec3f& a, float rot)
    {
        axis = a.unit();
        rotation = rot;

        x = axis(0) * sin(rotation);
        y = axis(1) * sin(rotation);
        z = axis(2) * sin(rotation);

        q0 = cos(rotation);
    }

    void setRotation(float rot)
    {
        rotation = rot;

        q0 = cos(rotation);
        x = axis(0) * sin(rotation);
        y = axis(1) * sin(rotation);
        z = axis(2) * sin(rotation);
    }


private:
    float q0;
    float x, y, z;

    vec3f axis;
    float rotation;
};


}