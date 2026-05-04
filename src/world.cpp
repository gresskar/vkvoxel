#include "world.hpp"

#include <algorithm>
#include <array>
#include <cmath>

static const std::array<Voxel, 24> BASE_CUBE_VERTICES = {{
    /* front face */
    Voxel { { -0.5f, -0.5f,  +0.5f }, { +1.0f, +1.0f, +1.0f }, { +0.0f, +0.0f } },
    Voxel { { +0.5f, -0.5f,  +0.5f }, { +1.0f, +1.0f, +1.0f }, { +1.0f, +0.0f } },
    Voxel { { +0.5f, +0.5f,  +0.5f }, { +1.0f, +1.0f, +1.0f }, { +1.0f, +1.0f } },
    Voxel { { -0.5f, +0.5f,  +0.5f }, { +1.0f, +1.0f, +1.0f }, { +0.0f, +1.0f } },

    /* back face */
    Voxel { { +0.5f, -0.5f, -0.5f }, { +1.0f, +1.0f, +1.0f }, { +0.0f, +0.0f } },
    Voxel { { -0.5f, -0.5f, -0.5f }, { +1.0f, +1.0f, +1.0f }, { +1.0f, +0.0f } },
    Voxel { { -0.5f, +0.5f, -0.5f }, { +1.0f, +1.0f, +1.0f }, { +1.0f, +1.0f } },
    Voxel { { +0.5f, +0.5f, -0.5f }, { +1.0f, +1.0f, +1.0f }, { +0.0f, +1.0f } },

    /* left face */
    Voxel { { -0.5f, -0.5f, -0.5f }, { +1.0f, +1.0f, +1.0f }, { +0.0f, +0.0f } },
    Voxel { { -0.5f, -0.5f, +0.5f }, { +1.0f, +1.0f, +1.0f }, { +1.0f, +0.0f } },
    Voxel { { -0.5f, +0.5f, +0.5f }, { +1.0f, +1.0f, +1.0f }, { +1.0f, +1.0f } },
    Voxel { { -0.5f, +0.5f, -0.5f }, { +1.0f, +1.0f, +1.0f }, { +0.0f, +1.0f } },

    /* right face */
    Voxel { { +0.5f, -0.5f, +0.5f }, { +1.0f, +1.0f, +1.0f }, { +0.0f, +0.0f } },
    Voxel { { +0.5f, -0.5f, -0.5f }, { +1.0f, +1.0f, +1.0f }, { +1.0f, +0.0f } },
    Voxel { { +0.5f, +0.5f, -0.5f }, { +1.0f, +1.0f, +1.0f }, { +1.0f, +1.0f } },
    Voxel { { +0.5f, +0.5f, +0.5f }, { +1.0f, +1.0f, +1.0f }, { +0.0f, +1.0f } },

    /* top face */
    Voxel { { -0.5f, +0.5f, +0.5f }, { +1.0f, +1.0f, +1.0f }, { +0.0f, +0.0f } },
    Voxel { { +0.5f, +0.5f, +0.5f }, { +1.0f, +1.0f, +1.0f }, { +1.0f, +0.0f } },
    Voxel { { +0.5f, +0.5f, -0.5f }, { +1.0f, +1.0f, +1.0f }, { +1.0f, +1.0f } },
    Voxel { { -0.5f, +0.5f, -0.5f }, { +1.0f, +1.0f, +1.0f }, { +0.0f, +1.0f } },

    /* bottom face */
    Voxel { { -0.5f, -0.5f, -0.5f }, { +1.0f, +1.0f, +1.0f }, { +0.0f, +0.0f } },
    Voxel { { +0.5f, -0.5f, -0.5f }, { +1.0f, +1.0f, +1.0f }, { +1.0f, +0.0f } },
    Voxel { { +0.5f, -0.5f, +0.5f }, { +1.0f, +1.0f, +1.0f }, { +1.0f, +1.0f } },
    Voxel { { -0.5f, -0.5f, +0.5f }, { +1.0f, +1.0f, +1.0f }, { +0.0f, +1.0f } }
}};

static const std::array<uint32_t, 36> BASE_CUBE_INDICES =
{
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20,
};

static float terrainHeight(const int x, const int z, const uint32_t maxHeight)
{
    const float height = 2.0f + std::sin(static_cast<float>(x) * 0.25f) * 1.75f + std::cos(static_cast<float>(z) * 0.3f) * 1.25f;
    
    const int result = static_cast<int>(std::floor(height));
    
    return static_cast<float>(std::clamp(result, 1, static_cast<int>(maxHeight)));
}

void World::appendCube(std::vector<Voxel> &vertices, std::vector<uint32_t> &indices, const glm::vec3 &origin)
{
    for (const Voxel &vertex : BASE_CUBE_VERTICES)
    {
        Voxel transformed = vertex;
        transformed.position += origin;
        vertices.push_back(transformed);
    }

    for (const auto index : BASE_CUBE_INDICES)
    {
        indices.push_back(static_cast<uint32_t>(vertices.size()) + index);
    }
}

void World::generateTerrain(const uint32_t width, const uint32_t depth, const uint32_t maxHeight)
{
    vertices.clear();
    indices.clear();

    vertices.reserve(width * depth * maxHeight * 24);
    indices.reserve(width * depth * maxHeight * 36);

    const float xOffset = static_cast<float>(width) * 0.5f;
    const float zOffset = static_cast<float>(depth) * 0.5f;

    for (uint32_t x = 0; x < width; ++x)
    {
        for (uint32_t z = 0; z < depth; ++z)
        {
            const float height = terrainHeight(static_cast<int>(x), static_cast<int>(z), maxHeight);

            for (uint32_t y = 0; y < static_cast<uint32_t>(height); ++y)
            {
                appendCube(vertices, indices, glm::vec3(static_cast<float>(x) - xOffset, static_cast<float>(y), static_cast<float>(z) - zOffset));
            }
        }
    }
}
