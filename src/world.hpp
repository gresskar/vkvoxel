#pragma once

#include "voxel.hpp"
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>

class World
{
public:
    std::vector<Voxel> vertices;
    std::vector<uint32_t> indices;

    void generateTerrain(const uint32_t width, const uint32_t depth, const uint32_t maxHeight);

private:
    static void appendCube(std::vector<Voxel> &vertices, std::vector<uint32_t> &indices, const glm::vec3 &origin);
};
