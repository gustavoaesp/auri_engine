#ifndef _NOISE_HPP_
#define _NOISE_HPP_

#include <stdint.h>
#include <vector>

namespace eng
{

class NoiseContext
{
public:
    NoiseContext(uint32_t seed);
    ~NoiseContext();

private:
    std::vector<uint8_t> array;
};

}

#endif