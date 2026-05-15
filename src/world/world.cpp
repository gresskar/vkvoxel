#include "world/world.hpp"

#include "world/chunk_generator.hpp"

#include <algorithm>
#include <cmath>
#include <exception>
#include <iterator>
#include <unordered_map>
#include <utility>

namespace
{
    class LoadedChunkBlockProvider final : public ChunkBlockProvider
    {
    public:
        explicit LoadedChunkBlockProvider(const std::unordered_map<ChunkCoord, Chunk> &chunks) : m_chunks(chunks){}

        [[nodiscard]] BlockType blockAt(const int32_t worldX, const int32_t y, const int32_t worldZ) const override
        {
            if (y < 0 || y >= static_cast<int32_t>(Chunk::HEIGHT))
            {
                return BlockType::Air;
            }

            const ChunkCoord coord = {
                .x = floorDiv(worldX, static_cast<int32_t>(Chunk::WIDTH)),
                .z = floorDiv(worldZ, static_cast<int32_t>(Chunk::DEPTH)),
            };

            const auto it = m_chunks.find(coord);
            if (it == m_chunks.end())
            {
                return BlockType::Air;
            }

            const uint32_t localX = static_cast<uint32_t>(worldX - coord.x * static_cast<int32_t>(Chunk::WIDTH));
            const uint32_t localZ = static_cast<uint32_t>(worldZ - coord.z * static_cast<int32_t>(Chunk::DEPTH));
            return it->second.get(localX, static_cast<uint32_t>(y), localZ);
        }

    private:
        [[nodiscard]] static int32_t floorDiv(const int32_t value, const int32_t divisor)
        {
            int32_t result = value / divisor;
            
            const int32_t remainder = value % divisor;
            
            if (remainder != 0 && ((remainder < 0) != (divisor < 0)))
            {
                --result;
            }

            return result;
        }

        const std::unordered_map<ChunkCoord, Chunk> &m_chunks;
    };

    [[nodiscard]] uint32_t chunkLodStep(const World::GenerationSettings &settings, uint32_t columnX, uint32_t columnZ)
    {
        if (!settings.enableLevelOfDetail)
        {
            return 1;
        }

        const float centerX = (static_cast<float>(settings.chunkColumnsX) - 1.0f) * 0.5f;
        const float centerZ = (static_cast<float>(settings.chunkColumnsZ) - 1.0f) * 0.5f;
        const float dx = static_cast<float>(columnX) - centerX;
        const float dz = static_cast<float>(columnZ) - centerZ;
        const float distance = std::sqrt(dx * dx + dz * dz);

        if (distance >= 10.0f)
        {
            return 4;
        }

        if (distance >= 5.0f)
        {
            return 2;
        }

        return 1;
    }

    void appendChunkMesh(World::Mesh &target, ChunkMesh source)
    {
        const uint32_t baseIndex = static_cast<uint32_t>(target.vertices.size());
        target.vertices.insert(target.vertices.end(), std::make_move_iterator(source.vertices.begin()), std::make_move_iterator(source.vertices.end()));
        target.indices.reserve(target.indices.size() + source.indices.size());

        for (const uint32_t index : source.indices)
        {
            target.indices.push_back(baseIndex + index);
        }
    }
}

World::~World()
{
    joinGenerationThread();
}

/* TODO: max height? */
void World::generateTerrain(const uint32_t width, const uint32_t depth)
{
    GenerationSettings settings{};
    settings.chunkColumnsX = std::max(1u, (width + CHUNK_WIDTH - 1) / CHUNK_WIDTH);
    settings.chunkColumnsZ = std::max(1u, (depth + CHUNK_DEPTH - 1) / CHUNK_DEPTH);
    settings.enableLevelOfDetail = false;

    Mesh mesh = generateChunkedTerrain(settings);
    vertices = std::move(mesh.vertices);
    indices = std::move(mesh.indices);
}

void World::requestChunkGeneration(const GenerationSettings &settings)
{
    if (m_generating.load())
    {
        return;
    }

    joinGenerationThread();
    m_generating.store(true);

    m_generationThread = std::thread([this, settings]() {
        try
        {
            Mesh mesh = generateChunkedTerrain(settings);
            {
                std::lock_guard lock(m_meshMutex);
                m_pendingMesh = std::move(mesh);
            }
        }
        catch (const std::exception &)
        {
        }

        m_generating.store(false);
    });
}

bool World::consumeGeneratedMesh(Mesh &mesh)
{
    std::lock_guard lock(m_meshMutex);
    if (!m_pendingMesh.has_value())
    {
        return false;
    }

    mesh = std::move(*m_pendingMesh);
    m_pendingMesh.reset();
    return true;
}

bool World::isGenerating(void) const
{
    return m_generating.load();
}

void World::joinGenerationThread(void)
{
    if (m_generationThread.joinable())
    {
        m_generationThread.join();
    }
}

World::Mesh World::generateChunkedTerrain(const GenerationSettings &settings)
{
    const uint32_t chunkColumnsX = std::max(1u, settings.chunkColumnsX);
    const uint32_t chunkColumnsZ = std::max(1u, settings.chunkColumnsZ);

    const int32_t startChunkX = -static_cast<int32_t>(chunkColumnsX / 2);
    const int32_t startChunkZ = -static_cast<int32_t>(chunkColumnsZ / 2);

    std::unordered_map<ChunkCoord, Chunk> chunks;
    chunks.reserve(static_cast<size_t>(chunkColumnsX) * static_cast<size_t>(chunkColumnsZ));

    ChunkGenerator generator(settings.seed);

    for (uint32_t columnZ = 0; columnZ < chunkColumnsZ; columnZ++)
    {
        for (uint32_t columnX = 0; columnX < chunkColumnsX; columnX++)
        {
            const ChunkCoord coord{
                .x = startChunkX + static_cast<int32_t>(columnX),
                .z = startChunkZ + static_cast<int32_t>(columnZ),
            };

            chunks.emplace(coord, generator.generate(coord));
        }
    }

    LoadedChunkBlockProvider blockProvider(chunks);
    
    ChunkMesher mesher{};
    
    Mesh combinedMesh{};
    combinedMesh.vertices.reserve(static_cast<size_t>(chunkColumnsX) * static_cast<size_t>(chunkColumnsZ) * 512);
    combinedMesh.indices.reserve(static_cast<size_t>(chunkColumnsX) * static_cast<size_t>(chunkColumnsZ) * 768);

    for (uint32_t columnZ = 0; columnZ < chunkColumnsZ; columnZ++)
    {
        for (uint32_t columnX = 0; columnX < chunkColumnsX; columnX++)
        {
            const ChunkCoord coord = {
                .x = startChunkX + static_cast<int32_t>(columnX),
                .z = startChunkZ + static_cast<int32_t>(columnZ),
            };

            const auto chunk = chunks.find(coord);
            if (chunk == chunks.end())
            {
                continue;
            }

            const uint32_t lodStep = chunkLodStep(settings, columnX, columnZ);
            appendChunkMesh(combinedMesh, mesher.mesh(chunk->second, blockProvider, ChunkMeshingOptions{
                .lodStep = lodStep,
                .positionOffset = glm::vec3{ 0.0f },
            }));
        }
    }

    return combinedMesh;
}
