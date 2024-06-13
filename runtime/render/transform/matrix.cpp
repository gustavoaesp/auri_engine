#include "render/transform/matrix.hpp"
#include "render/transform/vector.hpp"
#include <iostream>

namespace eng
{

mtx4f CreateTranslateMatrix(float x, float y, float z)
{
    return mtx4f(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
           x,    y,    z, 1.0f
    );
}

mtx4f CreateTranslateMatrix(const vec3f& trans)
{
    return CreateTranslateMatrix(trans(0), trans(1), trans(2));
}

mtx4f CreateScaleMatrix(float x, float y, float z)
{
    return mtx4f(
        x, 0.0f, 0.0f, 0.0f,
        0.0f, y, 0.0f, 0.0f,
        0.0f, 0.0f, z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

mtx4f CreateRotationMatrixX(float angle)
{
    return mtx4f(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cos(angle),-sin(angle), 0.0f,
        0.0f, sin(angle), cos(angle), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

mtx4f CreateRotationMatrixY(float angle)
{
    return mtx4f(
        cos(angle), 0.0f, sin(angle), 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        -sin(angle), 0.0f, cos(angle), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

mtx4f CreateRotationMatrixZ(float angle)
{
    return mtx4f(
        cos(angle),-sin(angle), 0.0f, 0.0f,
        sin(angle), cos(angle), 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

mtx4f CreatePerspectiveProjectionMatrix(float aspect_ratio, float fovy_degrees,
                                        float near_plane, float far_plane)
{
    float fovy = fovy_degrees / (180.0f / M_PI);
    return mtx4f(
        1.0f / (aspect_ratio * tan(fovy/2)), 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f / tan(fovy/2), 0.0f, 0.0f,
        0.0f, 0.0f, far_plane / (far_plane - near_plane), 1.0f,
        0.0f, 0.0f,  -far_plane*near_plane/(far_plane - near_plane), 0.0f
    );
}

void debug_print(const std::string& name, const vec3f& vec)
{
    std::cout << name << " = <"
        << vec(0) << ", " << vec(1) << ", " << vec(2) << ">\n";
}

mtx4f CreateViewMatrix(const vec3f& eye_pos, const vec3f& look_pos, const vec3f& up)
{
    vec3f zaxis = (look_pos - eye_pos).unit();
    vec3f xaxis = VectorCross(up, zaxis).unit();
    vec3f yaxis = VectorCross(zaxis, xaxis).unit();

    return mtx4f(
        xaxis(0), yaxis(0), zaxis(0), 0.0f,
        xaxis(1), yaxis(1), zaxis(1), 0.0f,
        xaxis(2), yaxis(2), zaxis(2), 0.0f,
        -vec3f::Dot(xaxis, eye_pos), -vec3f::Dot(yaxis, eye_pos), -vec3f::Dot(zaxis, eye_pos), 1.0f
    );
}

}
