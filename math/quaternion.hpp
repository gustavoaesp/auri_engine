#pragma once

#include <math.h>
#include "math/matrix.hpp"
#include "math/vector.hpp"

#include <algorithm>

namespace eng
{

class Quaternion
{
public:
    Quaternion():
        q0(0), x(0), y(1), z(0)
    {}
    Quaternion(float x, float y, float z, float q0):
        q0(q0), x(x), y(y), z(z)
    {}

    mtx4f toMatrix() const {

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

    Quaternion operator*(const Quaternion &a) const
    {
        return Quaternion(
            a.q0 *  x + a.x * q0 + a.y * z  - a.z * y,  // i
            a.q0 *  y - a.x * z  + a.y * q0 + a.z * x,  // j
            a.q0 *  z + a.x * y  - a.y * x  + a.z * q0,  // k
            a.q0 * q0 - a.x * x  - a.y * y  - a.z * z   // 1
        );
    }
    Quaternion operator+(const Quaternion &a) const
    {
        return Quaternion(
            x + a.x,
            y + a.y,
            z + a.z,
            q0 + a.q0
        );
    }
    Quaternion operator-(const Quaternion &a) const
    {
        return Quaternion(
            x - a.x,
            y - a.y,
            z - a.z,
            q0 - a.q0
        );
    }
    Quaternion operator*(float a) const
    {
        return Quaternion(
            x * a,
            y * a,
            z * a,
            q0 * a
        );
    }

    static float Dot(const Quaternion &a, const Quaternion &b)
    {
        float result = 0;
        result += a.x * b.x;
        result += a.y * b.y;
        result += a.z * b.z;
        result += a.q0 * b.q0;

        return result;
    }

    /*
    static Quaternion Slerp(const Quaternion &a, const Quaternion &b, float t)
    {
        float dotAB = Dot(a, b);
        Quaternion new_b = b;

        if (dotAB < 0.0f)
        {
            dotAB = -dotAB;
            new_b = Quaternion(-b.x, -b.y, -b.z, -b.q0);
        }

        float theta = acosf(dotAB);
        float sinTheta = sinf(theta);
        float af = sinf((1.0f - t) * theta) / sinTheta;
        float bf = sinf(t * theta) / sinTheta;


        return (a * af) + (new_b * bf);
    }

    static Quaternion SlerpFar(const Quaternion &a, const Quaternion &b, float t)
    {
        float dotAB = Dot(a, b);
        Quaternion new_b = b;

        if (dotAB > 0.0f)
        {
            dotAB = -dotAB;
            new_b = Quaternion(-b.x, -b.y, -b.z, -b.q0);
        }

        float theta = acosf(dotAB);
        float sinTheta = sinf(theta);
        float af = sinf((1.0f - t) * theta) / sinTheta;
        float bf = sinf(t * theta) / sinTheta;

        return (a * af) + (new_b * bf);
    }*/
    static Quaternion Slerp(const Quaternion &a, const Quaternion &b, float t)
    {
        // Dot product - the cosine of the angle between 2 vectors.
        float dot = Quaternion::Dot(a, b);
        // Clamp it to be in the range of Acos()
        // This may be unnecessary, but floating point
        // precision can be a fickle mistress.
        //clamp(dot, -1.0f, 1.0f);
        std::clamp(dot, -1.0f, 1.0f);
        // Acos(dot) returns the angle between start and end,
        // And multiplying that by percent returns the angle between
        // start and the final result.
        float theta = acosf(dot)*t;
        Quaternion relative = (b - a)*dot;
        relative = relative.Normalize();
        // Orthonormal basis
        // The final result.
        return ((a*cosf(theta)) + (relative*sinf(theta)));
    }

    static Quaternion Lerp(const Quaternion &a, const Quaternion &b, float t)
    {
        return a * (1.0f - t) + b * t;
    }

    Quaternion Normalize() const
    {
        float mag = sqrt(x*x + y*y  + z*z + q0 * q0);
        return Quaternion(
            x / mag,
            y / mag,
            z / mag,
            q0/ mag
        );
    }
private:
    float q0;
    float x, y, z;

    vec3f axis;
    float rotation;

};

}