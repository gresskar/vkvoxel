#pragma once

#include <glm/glm.hpp>
#include <volk/volk.h>
#include <array>

struct Rectangle
{
    glm::vec2 position;
    glm::vec3 color;

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

    /* An attribute description struct describes how to extract a vertex attribute from a chunk of vertex data originating from a binding description. We have two attributes, position and color, so we need two attribute description structs. */
    constexpr static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions(void)
    {
        constexpr std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {{
            /* Position */
            VkVertexInputAttributeDescription{
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(Rectangle, position),
            },
            
            /* Color */
            VkVertexInputAttributeDescription{
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Rectangle, color),
            }
        }};

        return attributeDescriptions;
    };
};

/* Vertices are normally stored on the heap, but it's fine to store this on stack since it's so small */
constexpr std::array<Rectangle, 4> verticesRectangle = {{
    { .position = { -0.5f, -0.5f }, .color = { 1.0f, 0.0f, 0.0f } },
    { .position = { +0.5f, -0.5f }, .color = { 0.0f, 1.0f, 0.0f } },
    { .position = { +0.5f, +0.5f }, .color = { 0.0f, 0.0f, 1.0f } },
    { .position = { -0.5f, +0.5f }, .color = { 1.0f, 1.0f, 1.0f } },
}};

constexpr std::array<uint8_t, 6> indicesRectangle = {
    0, 1, 2, 2, 3, 0
};