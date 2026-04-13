#pragma once

#include <glm/glm.hpp>
#include <volk/volk.h>
#include <array>

struct Voxel
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec3 texCoord;

     /* the number of bytes between data entries and whether to move to the next data entry after each vertex or after each instance */
    constexpr static VkVertexInputBindingDescription getBindingDescription(void)
    {
        constexpr VkVertexInputBindingDescription bindingDescription = {
            .binding = 0,
            .stride = sizeof(Voxel),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };

        return bindingDescription;
    }

    /* An attribute description struct describes how to extract a vertex attribute from a chunk of vertex data originating from a binding description. We have two attributes, position and color, so we need two attribute description structs. */
    constexpr static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions(void)
    {
        constexpr std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {{
            /* Position */
            VkVertexInputAttributeDescription{
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(Voxel, position),
            },
            
            /* Color */
            VkVertexInputAttributeDescription{
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Voxel, color),
            },

            /* Texture coordinate */
            VkVertexInputAttributeDescription{
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(Voxel, texCoord),
            }
        }};

        return attributeDescriptions;
    };
};

/* Vertices are normally stored on the heap, but it's fine to store this on stack since it's so small */
constexpr std::array<Voxel, 24> verticesVoxel = {{
    { .position = { +0.5f, +0.0f, +0.0f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },
    { .position = { +0.5f, +0.5f, +0.0f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },
    { .position = { +0.5f, +0.5f, +0.5f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },
    { .position = { +0.5f, +0.0f, +0.5f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },

    { .position = { +0.0f, +0.0f, +0.0f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },
    { .position = { +0.0f, +0.0f, +0.5f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },
    { .position = { +0.0f, +0.5f, +0.5f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },
    { .position = { +0.5f, +0.0f, +0.5f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },

    { .position = { +0.0f, +0.5f, +0.0f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },
    { .position = { +0.0f, +0.5f, +0.5f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },
    { .position = { +0.5f, +0.5f, +0.5f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },
    { .position = { +0.5f, +0.5f, +0.0f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },

    { .position = { +0.0f, +0.0f, +0.0f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },
    { .position = { +0.5f, +0.0f, +0.0f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },
    { .position = { +0.5f, +0.0f, +0.5f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },
    { .position = { +0.0f, +0.0f, +0.5f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },

    { .position = { +0.0f, +0.0f, +0.5f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },
    { .position = { +0.5f, +0.0f, +0.5f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },
    { .position = { +0.5f, +0.5f, +0.5f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },
    { .position = { +0.0f, +0.5f, +0.5f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },

    { .position = { +0.0f, +0.0f, +0.0f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },
    { .position = { +0.0f, +0.5f, +0.0f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },
    { .position = { +0.5f, +0.5f, +0.0f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },
    { .position = { +0.5f, +0.0f, +0.0f }, .color = { 1.0f, +0.0f, +0.0f }, .texCoord = { +0.0f, +0.0f, +0.0f } },

}};

constexpr std::array<uint8_t, 36> indicesVoxel = {
    0,1,2, 0,2,3,
    4,5,6, 4,6,7,
    8,9,10, 8,10,11,
    12,13,14, 12,14,15,
    16,17,18, 16,18,19,
    20,21,22, 20,22,23
};