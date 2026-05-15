#pragma once

#include "world/chunk.hpp"

#include "FastNoiseLite.h"

#include <cstdint>

class ChunkGenerator
{
public:
    explicit ChunkGenerator(const int32_t seed);

    [[nodiscard]] Chunk generate(const ChunkCoord coord) const;

private:
    const uint32_t SEA_LEVEL = 22;

    int32_t m_seed = 0;

    [[nodiscard]] float normalizedNoise(const FastNoiseLite &noise, const float x, const float z) const
    {
        return (noise.GetNoise(x, z) + 1.0f) * 0.5f;
    }
};
