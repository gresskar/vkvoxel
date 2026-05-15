#pragma once

#include "renderer/voxel.hpp"
#include "world/chunk.hpp"

#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

struct ChunkMesh
{
    ChunkCoord coord{};
    uint32_t lodStep = 1;
    std::vector<Voxel> vertices{};
    std::vector<uint32_t> indices{};

    [[nodiscard]] bool empty(void) const
    {
        return vertices.empty() || indices.empty();
    }
};

class ChunkBlockProvider
{
public:
    virtual ~ChunkBlockProvider() = default;

    [[nodiscard]] virtual BlockType blockAt(const int32_t worldX, const int32_t y, const int32_t worldZ) const = 0;
};

struct ChunkMeshingOptions
{
    uint32_t lodStep = 1;
    glm::vec3 positionOffset{ 0.0f };
};

class ChunkMesher
{
public:
    [[nodiscard]] ChunkMesh mesh(const Chunk &chunk, const ChunkBlockProvider &blocks, const ChunkMeshingOptions &options = {}) const;
};
