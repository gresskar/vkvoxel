#include "world/chunk_mesher.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

namespace
{
    constexpr float ATLAS_WIDTH = 64.0f;
    constexpr float ATLAS_HEIGHT = 48.0f;
    constexpr float ATLAS_TILE_SIZE = 16.0f;
    constexpr float ATLAS_TEXEL_PADDING = 0.5f;

    [[nodiscard]] uint32_t sanitizeLodStep(uint32_t lodStep)
    {
        if (lodStep >= 4 && Chunk::WIDTH % 4 == 0 && Chunk::DEPTH % 4 == 0 && Chunk::HEIGHT % 4 == 0)
        {
            return 4;
        }

        if (lodStep >= 2 && Chunk::WIDTH % 2 == 0 && Chunk::DEPTH % 2 == 0 && Chunk::HEIGHT % 2 == 0)
        {
            return 2;
        }

        return 1;
    }

    [[nodiscard]] glm::vec4 texAtlas(uint32_t tileX, uint32_t tileY)
    {
        return glm::vec4 {
            (static_cast<float>(tileX) * ATLAS_TILE_SIZE + ATLAS_TEXEL_PADDING) / ATLAS_WIDTH,
            (static_cast<float>(tileY) * ATLAS_TILE_SIZE + ATLAS_TEXEL_PADDING) / ATLAS_HEIGHT,
            (ATLAS_TILE_SIZE - ATLAS_TEXEL_PADDING * 2.0f) / ATLAS_WIDTH,
            (ATLAS_TILE_SIZE - ATLAS_TEXEL_PADDING * 2.0f) / ATLAS_HEIGHT,
        };
    }

    [[nodiscard]] glm::vec4 texAtlasForFace(const BlockType blockType, const uint32_t axis, bool isPositive)
    {
        switch (blockType)
        {
            case BlockType::Grass:
                if (axis == 1 && isPositive)
                {
                    return texAtlas(2, 0);
                }

                if (axis == 1 && !isPositive)
                {
                    return texAtlas(0, 0);
                }
                else
                {
                    return texAtlas(1, 0);
                }
            
            case BlockType::Sand:
                return texAtlas(3, 0);
            
            case BlockType::Dirt:
                return texAtlas(0, 0);
            
            case BlockType::Stone:
                return texAtlas(0, 1);
            
            case BlockType::Air:
                break;

            default:
                break;
        }

        return texAtlas(0, 0);
    }

    [[nodiscard]] glm::vec3 faceShade(const uint32_t axis, const bool isPositive)
    {
        float shade = 0.75;

        switch (axis)
        {
            case 0:
                shade = isPositive ? 0.75f : 0.60f;
                break;

            case 1:
                shade = isPositive ? 1.0f : 0.40f;
                break;
            
            default:
                shade = isPositive ? 0.80f : 0.65f;
        }

        return glm::vec3{ shade };
    }

    [[nodiscard]] BlockType dominantBlockInRegion(const ChunkBlockProvider &blocks, int32_t x, int32_t y, int32_t z, uint32_t size)
    {
        std::array<uint32_t, BLOCK_TYPE_COUNT> counts{};

        for (uint32_t dy = 0; dy < size; dy++)
        {
            for (uint32_t dz = 0; dz < size; dz++)
            {
                for (uint32_t dx = 0; dx < size; dx++)
                {
                    const BlockType block = blocks.blockAt(x + static_cast<int32_t>(dx), y + static_cast<int32_t>(dy), z + static_cast<int32_t>(dz));
                    ++counts.at(static_cast<size_t>(block));
                }
            }
        }

        size_t bestIndex = static_cast<size_t>(BlockType::Air);
        uint32_t bestCount = 0;
        for (size_t i = 1; i < counts.size(); ++i)
        {
            if (counts.at(i) > bestCount)
            {
                bestCount = counts.at(i);
                bestIndex = i;
            }
        }

        return bestCount == 0 ? BlockType::Air : static_cast<BlockType>(bestIndex);
    }

    void appendGreedyQuad
    (
        ChunkMesh &mesh,
        BlockType block,
        uint32_t axis,
        bool isPositive,
        uint32_t plane,
        uint32_t u0,
        uint32_t v0,
        uint32_t u1,
        uint32_t v1,
        uint32_t step,
        const Chunk &chunk,
        const glm::vec3 &positionOffset
    )
    {
        const uint32_t uAxis = (axis + 1) % 3;
        const uint32_t vAxis = (axis + 2) % 3;
        
        auto makePosition = [&](const uint32_t axisCoordinate, const uint32_t uCoordinate, const uint32_t vCoordinate)
        {
            std::array<float, 3> coordinates{};
            coordinates.at(axis) = static_cast<float>(axisCoordinate * step);
            coordinates.at(uAxis) = static_cast<float>(uCoordinate * step);
            coordinates.at(vAxis) = static_cast<float>(vCoordinate * step);

            return glm::vec3{
                static_cast<float>(chunk.minBlockX()) + coordinates.at(0),
                coordinates.at(1),
                static_cast<float>(chunk.minBlockZ()) + coordinates.at(2),
            } + positionOffset;
        };

        auto makeTexCoord = [&](const glm::vec3 &position)
        {
            if (axis == 0)
            {
                return glm::vec2{ position.z, -position.y };
            }

            if (axis == 2)
            {
                return glm::vec2{ position.x, -position.y };
            }

            return glm::vec2{ position.x, position.z };
        };

        std::array<glm::vec3, 4> positions{};
        std::array<glm::vec2, 4> texCoords{};

        if (isPositive)
        {
            positions = {
                makePosition(plane, u0, v0),
                makePosition(plane, u1, v0),
                makePosition(plane, u1, v1),
                makePosition(plane, u0, v1)
            };
        }
        else
        {
            positions = {
                makePosition(plane, u0, v0),
                makePosition(plane, u0, v1),
                makePosition(plane, u1, v1),
                makePosition(plane, u1, v0)
            };
        }

        for (size_t i = 0; i < positions.size(); ++i)
        {
            texCoords.at(i) = makeTexCoord(positions.at(i));
        }

        const glm::vec3 color = faceShade(axis, isPositive);
        const glm::vec4 atlas = texAtlasForFace(block, axis, isPositive);
        const uint32_t baseIndex = static_cast<uint32_t>(mesh.vertices.size());
        for (size_t i = 0; i < positions.size(); ++i)
        {
            mesh.vertices.push_back(Voxel{
                .position = positions.at(i),
                .color = color,
                .texCoord = texCoords.at(i),
                .texAtlas = atlas,
            });
        }

        mesh.indices.push_back(baseIndex + 0);
        mesh.indices.push_back(baseIndex + 1);
        mesh.indices.push_back(baseIndex + 2);
        mesh.indices.push_back(baseIndex + 2);
        mesh.indices.push_back(baseIndex + 3);
        mesh.indices.push_back(baseIndex + 0);
    }

    void appendGreedyFacesForAxis
    (
        ChunkMesh &mesh,
        const std::vector<BlockType> &lodBlocks,
        const ChunkBlockProvider &blocks,
        const Chunk &chunk,
        uint32_t axis,
        bool positive,
        uint32_t lodWidth,
        uint32_t lodHeight,
        uint32_t lodDepth,
        const ChunkMeshingOptions &options
    )
    {
        const std::array<uint32_t, 3> dims = { lodWidth, lodHeight, lodDepth };
        const uint32_t uAxis = (axis + 1) % 3;
        const uint32_t vAxis = (axis + 2) % 3;
        const uint32_t axisLength = dims.at(axis);
        const uint32_t uLength = dims.at(uAxis);
        const uint32_t vLength = dims.at(vAxis);

        auto localIndex = [&](const uint32_t x, const uint32_t y, const uint32_t z)
        {
            return static_cast<size_t>(x) + static_cast<size_t>(lodWidth) * (static_cast<size_t>(z) + static_cast<size_t>(lodDepth) * static_cast<size_t>(y));
        };

        auto cellAt = [&](const int32_t x, const int32_t y, const int32_t z)
        {
            if (x >= 0 && y >= 0 && z >= 0 &&
                x < static_cast<int32_t>(lodWidth) &&
                y < static_cast<int32_t>(lodHeight) &&
                z < static_cast<int32_t>(lodDepth))
            {
                return lodBlocks.at(localIndex(static_cast<uint32_t>(x), static_cast<uint32_t>(y), static_cast<uint32_t>(z)));
            }

            const int32_t worldX = chunk.minBlockX() + x * static_cast<int32_t>(options.lodStep);
            const int32_t worldY = y * static_cast<int32_t>(options.lodStep);
            const int32_t worldZ = chunk.minBlockZ() + z * static_cast<int32_t>(options.lodStep);
            return dominantBlockInRegion(blocks, worldX, worldY, worldZ, options.lodStep);
        };

        std::vector<BlockType> mask(static_cast<size_t>(uLength) * static_cast<size_t>(vLength), BlockType::Air);

        for (uint32_t plane = 0; plane <= axisLength; plane++)
        {
            std::fill(mask.begin(), mask.end(), BlockType::Air);

            for (uint32_t v = 0; v < vLength; ++v)
            {
                for (uint32_t u = 0; u < uLength; ++u)
                {
                    std::array<int32_t, 3> solidCoordinates{};
                    solidCoordinates.at(axis) = positive ? static_cast<int32_t>(plane) - 1 : static_cast<int32_t>(plane);
                    solidCoordinates.at(uAxis) = static_cast<int32_t>(u);
                    solidCoordinates.at(vAxis) = static_cast<int32_t>(v);

                    std::array<int32_t, 3> neighborCoordinates = solidCoordinates;
                    neighborCoordinates.at(axis) += positive ? 1 : -1;

                    const BlockType block = cellAt(solidCoordinates.at(0), solidCoordinates.at(1), solidCoordinates.at(2));

                    if (block == BlockType::Air)
                    {
                        continue;
                    }

                    if (cellAt(neighborCoordinates.at(0), neighborCoordinates.at(1), neighborCoordinates.at(2)) == BlockType::Air)
                    {
                        mask.at(static_cast<size_t>(u) + static_cast<size_t>(uLength) * static_cast<size_t>(v)) = block;
                    }
                }
            }

            for (uint32_t v = 0; v < vLength; ++v)
            {
                for (uint32_t u = 0; u < uLength;)
                {
                    const BlockType block = mask.at(static_cast<size_t>(u) + static_cast<size_t>(uLength) * static_cast<size_t>(v));
                    if (block == BlockType::Air)
                    {
                        ++u;
                        continue;
                    }

                    uint32_t width = 1;
                    while (u + width < uLength && mask.at(static_cast<size_t>(u + width) + static_cast<size_t>(uLength) * static_cast<size_t>(v)) == block)
                    {
                        width++;
                    }

                    uint32_t height = 1;
                    bool canGrow = true;
                    while (v + height < vLength && canGrow)
                    {
                        for (uint32_t scanU = 0; scanU < width; scanU++)
                        {
                            if (mask.at(static_cast<size_t>(u + scanU) + static_cast<size_t>(uLength) * static_cast<size_t>(v + height)) != block)
                            {
                                canGrow = false;
                                break;
                            }
                        }

                        if (canGrow)
                        {
                            ++height;
                        }
                    }

                    appendGreedyQuad(mesh, block, axis, positive, plane, u, v, u + width, v + height, options.lodStep, chunk, options.positionOffset);

                    for (uint32_t clearV = 0; clearV < height; clearV++)
                    {
                        for (uint32_t clearU = 0; clearU < width; clearU++)
                        {
                            mask.at(static_cast<size_t>(u + clearU) + static_cast<size_t>(uLength) * static_cast<size_t>(v + clearV)) = BlockType::Air;
                        }
                    }

                    u += width;
                }
            }
        }
    }
}

ChunkMesh ChunkMesher::mesh(const Chunk &chunk, const ChunkBlockProvider &blocks, const ChunkMeshingOptions &options) const
{
    const uint32_t step = sanitizeLodStep(options.lodStep);
    const uint32_t lodWidth = Chunk::WIDTH / step;
    const uint32_t lodDepth = Chunk::DEPTH / step;
    const uint32_t lodHeight = Chunk::HEIGHT / step;

    ChunkMesh mesh{};
    mesh.coord = chunk.coord();
    mesh.lodStep = step;
    mesh.vertices.reserve(512);
    mesh.indices.reserve(768);

    std::vector<BlockType> lodBlocks(static_cast<size_t>(lodWidth) * static_cast<size_t>(lodDepth) * static_cast<size_t>(lodHeight), BlockType::Air);
    auto localIndex = [&](uint32_t x, uint32_t y, uint32_t z) {
        return static_cast<size_t>(x) + static_cast<size_t>(lodWidth) * (static_cast<size_t>(z) + static_cast<size_t>(lodDepth) * static_cast<size_t>(y));
    };

    for (uint32_t y = 0; y < lodHeight; ++y)
    {
        for (uint32_t z = 0; z < lodDepth; ++z)
        {
            for (uint32_t x = 0; x < lodWidth; ++x)
            {
                lodBlocks.at(localIndex(x, y, z)) = dominantBlockInRegion(
                    blocks,
                    chunk.minBlockX() + static_cast<int32_t>(x * step),
                    static_cast<int32_t>(y * step),
                    chunk.minBlockZ() + static_cast<int32_t>(z * step),
                    step);
            }
        }
    }

    ChunkMeshingOptions sanitizedOptions = options;
    sanitizedOptions.lodStep = step;

    for (uint32_t axis = 0; axis < 3; axis++)
    {
        appendGreedyFacesForAxis(mesh, lodBlocks, blocks, chunk, axis, true, lodWidth, lodHeight, lodDepth, sanitizedOptions);
        appendGreedyFacesForAxis(mesh, lodBlocks, blocks, chunk, axis, false, lodWidth, lodHeight, lodDepth, sanitizedOptions);
    }

    return mesh;
}
