#pragma once

#include "world/block.hpp"
#include "world/chunk.hpp"
#include "world/chunk_mesher.hpp"

#include <atomic>
#include <cstdint>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

class World
{
public:
    static constexpr uint32_t CHUNK_WIDTH = Chunk::WIDTH;
    static constexpr uint32_t CHUNK_DEPTH = Chunk::DEPTH;
    static constexpr uint32_t CHUNK_HEIGHT = Chunk::HEIGHT;

    using BlockType = ::BlockType;
    using ChunkCoord = ::ChunkCoord;
    using Mesh = ChunkMesh;

    struct GenerationSettings
    {
        int32_t seed = 1337;
        uint32_t chunkColumnsX = 32;
        uint32_t chunkColumnsZ = 16;
        bool enableLevelOfDetail = false;
    };

    std::vector<Voxel> vertices{};
    std::vector<uint32_t> indices{};

    World() = default;
    ~World();

    World(const World &) = delete;
    World &operator=(const World &) = delete;
    World(World &&) = delete;
    World &operator=(World &&) = delete;

    void generateTerrain(const uint32_t width, const uint32_t depth);
    void requestChunkGeneration(const GenerationSettings &settings);
    [[nodiscard]] bool consumeGeneratedMesh(Mesh &mesh);
    [[nodiscard]] bool isGenerating(void) const;

private:
    void joinGenerationThread(void);
    static Mesh generateChunkedTerrain(const GenerationSettings &settings);

    std::thread m_generationThread;
    mutable std::mutex m_meshMutex;
    std::optional<Mesh> m_pendingMesh;
    std::atomic_bool m_generating{ false };
};
