#pragma once

#include <glm/glm.hpp>
#include <volk/volk.h>
#include <array>

struct Rectangle
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;

    /* the number of bytes between data entries and whether to move to the next data entry after each vertex or after each instance */
    constexpr static VkVertexInputBindingDescription getBindingDescription(void)
    {
        constexpr VkVertexInputBindingDescription bindingDescription = {
            .binding = 0,
            .stride = sizeof(Rectangle),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };

        return bindingDescription;
    }

    /* An attribute description struct describes how to extract a vertex attribute from a chunk of vertex data originating from a binding description. We have three attributes, position, color and texture coordinates, so we need three attribute description structs. */
    constexpr static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions(void)
    {
        constexpr std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {{
            /* Position */
            VkVertexInputAttributeDescription{
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Rectangle, position),
            },
            
            /* Color */
            VkVertexInputAttributeDescription{
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Rectangle, color),
            },

            /* Texture coordinates */
            VkVertexInputAttributeDescription{
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(Rectangle, texCoord),
            }
        }};

        return attributeDescriptions;
    };
};

/* Vertices are normally stored on the heap, but it's fine to store this on stack since it's so small */
constexpr std::array<Rectangle, 8> verticesRectangle = {{
    { .position = { -0.5f, -0.5f, +0.0f }, .color = { 1.0f, 0.0f, 0.0f }, .texCoord = { 0.0f, 0.0f } },
    { .position = { +0.5f, -0.5f, +0.0f }, .color = { 0.0f, 1.0f, 0.0f }, .texCoord = { 1.0f, 0.0f } },
    { .position = { +0.5f, +0.5f, +0.0f }, .color = { 0.0f, 0.0f, 1.0f }, .texCoord = { 1.0f, 1.0f } },
    { .position = { -0.5f, +0.5f, +0.0f }, .color = { 1.0f, 1.0f, 1.0f }, .texCoord = { 0.0f, 1.0f } },

    { .position = { -0.5f, -0.5f, -0.5f }, .color = { 1.0f, 0.0f, 0.0f }, .texCoord = { 0.0f, 0.0f } },
    { .position = { +0.5f, -0.5f, -0.5f }, .color = { 0.0f, 1.0f, 0.0f }, .texCoord = { 1.0f, 0.0f } },
    { .position = { +0.5f, +0.5f, -0.5f }, .color = { 0.0f, 0.0f, 1.0f }, .texCoord = { 1.0f, 1.0f } },
    { .position = { -0.5f, +0.5f, -0.5f }, .color = { 1.0f, 1.0f, 1.0f }, .texCoord = { 0.0f, 1.0f } }
}};

constexpr std::array<uint8_t, 12> indicesRectangle = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
};