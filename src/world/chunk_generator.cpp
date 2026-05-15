#include "world/chunk_generator.hpp"

#include "FastNoiseLite.h"

#include <algorithm>
#include <cmath>



ChunkGenerator::ChunkGenerator(const int32_t seed) : m_seed(seed){}

Chunk ChunkGenerator::generate(const ChunkCoord coord) const
{
    Chunk chunk(coord);

    FastNoiseLite elevationNoise(m_seed);
    elevationNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    elevationNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    elevationNoise.SetFractalOctaves(5);
    elevationNoise.SetFractalLacunarity(2.0f);
    elevationNoise.SetFractalGain(0.5f);
    elevationNoise.SetFrequency(0.0065f);

    FastNoiseLite detailNoise(m_seed ^ 0x5bd1e995);
    detailNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);
    detailNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    detailNoise.SetFractalOctaves(3);
    detailNoise.SetFrequency(0.035f);

    FastNoiseLite beachNoise(m_seed ^ 0x27d4eb2d);
    beachNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    beachNoise.SetFrequency(0.018f);

    for (uint32_t z = 0; z < Chunk::DEPTH; z++)
    {
        for (uint32_t x = 0; x < Chunk::WIDTH; x++)
        {
            const float worldX = static_cast<float>(chunk.minBlockX() + static_cast<int32_t>(x));
            const float worldZ = static_cast<float>(chunk.minBlockZ() + static_cast<int32_t>(z));

            const float elevation = normalizedNoise(elevationNoise, worldX, worldZ);
            
            const float detail = detailNoise.GetNoise(worldX, worldZ);
                        
            const float beach = normalizedNoise(beachNoise, worldX, worldZ);

            const float ridges = 1.0f - std::abs(detailNoise.GetNoise(worldX * 0.35f + 90.0f, worldZ * 0.35f - 42.0f));
            const float surface = 10.0f + elevation * 32.0f + ridges * 12.0f + detail * 4.0f;

            const uint32_t surfaceY = static_cast<uint32_t>(std::clamp(static_cast<int32_t>(std::floor(surface)), 1, static_cast<int32_t>(Chunk::HEIGHT - 2)));
            const bool sandySurface = surfaceY <= SEA_LEVEL + 2 || (surfaceY <= SEA_LEVEL + 5 && beach > 0.50f);

            for (uint32_t y = 0; y <= surfaceY; y++)
            {
                const uint32_t depthFromSurface = surfaceY - y;
                
                BlockType block = BlockType::Stone;

                if (depthFromSurface == 0)
                {
                    block = sandySurface ? BlockType::Sand : BlockType::Grass;
                }
                else if (depthFromSurface <= 4)
                {
                    block = sandySurface ? BlockType::Sand : BlockType::Dirt;
                }

                chunk.set(x, y, z, block);
            }
        }
    }

    chunk.clearDirty();
    return chunk;
}
