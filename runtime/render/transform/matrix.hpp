#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include <math/matrix.hpp>
#include <math/vector.hpp>

namespace eng
{

mtx4f CreateTranslateMatrix(float x, float y, float z);
mtx4f CreateTranslateMatrix(const vec3f& trans);
mtx4f CreateScaleMatrix(float x, float y, float z);

// Rotation
mtx4f CreateRotationMatrixX(float angle);
mtx4f CreateRotationMatrixY(float angle);
mtx4f CreateRotationMatrixZ(float angle);

mtx4f CreatePerspectiveProjectionMatrix(float aspect_ratio, float fovy_degrees,
                                        float near_plane, float far_plane);

mtx4f CreateViewMatrix(const vec3f& eye_pos, const vec3f& look_pos, const vec3f& up);

}
