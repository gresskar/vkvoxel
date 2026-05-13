#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <volk/volk.h>
#include <array>

struct Voxel
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;

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

    /* An attribute description struct describes how to extract a vertex attribute from a chunk of vertex data originating from a binding description */
    constexpr static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions(void)
    {
        constexpr std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {{
            /* Position */
            VkVertexInputAttributeDescription{
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
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

    bool operator==(const Voxel &other) const
    {
        return position == other.position && color == other.color && texCoord == other.texCoord;
    }
};

template <>
struct std::hash<Voxel>
{
	size_t operator()(Voxel const &vertex) const noexcept
	{
		return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
	}
};
