#include "render/transform/vector.hpp"

namespace eng
{

vec3f VectorCross(const vec3f& a, const vec3f& b)
{
    return vec3f(
        a(1) * b(2) - a(2)*b(1),
        a(2) * b(0) - a(0)*b(2),
        a(0) * b(1) - a(1)*b(0)
    );
}

}
