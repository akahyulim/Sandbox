#pragma once
#include <random>
#include <cstdint>

#define DV_DELETE(p) { if(p) delete (p); (p) = nullptr; }
#define DV_DELETE_ARRAY(p) { if(p) delete[](p); (p) = nullptr; }
#define DV_RELEASE(p) { if(p) (p)->Release(); (p) = nullptr; }

namespace Dive
{
    inline uint64_t GenerateUniqueID(uint64_t min = 0, uint64_t max = UINT64_MAX)
    {
        static std::mt19937_64 engine{ std::random_device{}() };
        static std::uniform_int_distribution<uint64_t> dist(min, max);
        return dist(engine);
    }
}