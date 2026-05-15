#pragma once

#include <SDL3/SDL_video.h>
#include <volk/volk.h>
#include <cstdint>
#include <string>
#include <vector>

#include "camera/camera.hpp"
#include "renderer/voxel.hpp"
#include "world/world.hpp"

class Renderer
{
public:
    struct QueueFamilyIndices
    {
        uint32_t graphicsFamily = UINT32_MAX;
        uint32_t presentFamily = UINT32_MAX;

        [[nodiscard]] bool isComplete(void) const
        {
            return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX;
        }
    };

    void init(SDL_Window *window, const World &world);
    void cleanup(void);

    void drawFrame(void);
    void updateWorldMesh(World::Mesh mesh);
    void updateUniformBuffer(const Camera &camera);
    void setFramebufferResized(bool resized);
    void waitIdle(void) const;

private:
    static constexpr uint8_t MAX_FRAMES_IN_FLIGHT = 2;

    SDL_Window *m_window = nullptr;

    void loadVulkan(void);    
    
    void createInstance(void);
    VkInstance m_instance{ VK_NULL_HANDLE };
    
    void createSurface(SDL_Window *window);
    VkSurfaceKHR m_surface{ VK_NULL_HANDLE };
    
    QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice &device) const;
    QueueFamilyIndices m_queueFamilyIndices{};

    void pickPhysicalDevice(void);
    VkPhysicalDevice m_physicalDevice{ VK_NULL_HANDLE };
    
    void createLogicalDevice(void);
    VkDevice m_device{ VK_NULL_HANDLE };
    VkQueue m_graphicsQueue{ VK_NULL_HANDLE };
    VkQueue m_presentQueue{ VK_NULL_HANDLE };
    
    void createSwapChain(void);
    void cleanupSwapChain(void);
    void recreateSwapChain(void);
    VkSwapchainKHR m_swapChain{ VK_NULL_HANDLE };
    VkSurfaceCapabilitiesKHR m_swapChainCapabilities{};
    VkSurfaceFormatKHR m_swapChainSurfaceFormat{};
    VkFormat m_swapChainImageFormat;
    VkPresentModeKHR m_swapChainPresentMode{};
    VkExtent2D m_swapChainExtent{};
    std::vector<VkImage> m_swapChainImages{};

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void createImageViews(void);
    std::vector<VkImageView> m_swapChainImageViews{};

    void createDescriptorSetLayout(void);
    VkDescriptorSetLayout m_descriptorSetLayout{ VK_NULL_HANDLE };
    
    [[nodiscard]] std::vector<char> readFile(const std::string &filename) const;
    [[nodiscard]] VkShaderModule createShaderModule(const std::vector<char> &shaderCode) const;
    void createGraphicsPipeline(void);
    VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };
    VkPipeline m_graphicsPipeline{ VK_NULL_HANDLE };

    void createCommandPool(void);
    VkCommandPool m_cmdPool{ VK_NULL_HANDLE };
    
    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
    VkFormat findDepthFormat(void) const;
    bool hasStencilComponent(const VkFormat &format) const;
    void createDepthResources(void);
    VkImage m_depthImage{ VK_NULL_HANDLE };
    VkDeviceMemory m_depthImageMemory{ VK_NULL_HANDLE };
    VkImageView m_depthImageView{ VK_NULL_HANDLE };

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);
    void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer srcBuffer, VkImage image, uint32_t width, uint32_t height);
    void createTextureImage(void);
    VkImage m_textureImage{ VK_NULL_HANDLE };
    VkDeviceMemory m_textureImageMemory{ VK_NULL_HANDLE };

    void createTextureImageView(void);
    VkImageView m_textureImageView{ VK_NULL_HANDLE };

    void createTextureSampler();
    VkSampler m_textureSampler{ VK_NULL_HANDLE };

    VkCommandBuffer beginSingleTimeCommands(void);
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
   
    void loadModel(const World &world);
    std::vector<Voxel> m_vertices;
    std::vector<uint32_t> m_indices;

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkDeviceSize &size);
    void destroyGeometryBuffers(void);
    
    void createVertexBuffer(void);
    VkBuffer m_vertexBuffer{ VK_NULL_HANDLE };
    VkDeviceMemory m_vertexBufferMemory{ VK_NULL_HANDLE };

    void createIndexBuffer(void);
    VkDeviceMemory m_indexBufferMemory{ VK_NULL_HANDLE };
    VkBuffer m_indexBuffer{ VK_NULL_HANDLE };
    
    void createUniformBuffers(void);
    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;
    std::vector<void *> m_uniformBuffersMapped;

    void createDescriptorPool(void);
    VkDescriptorPool m_descriptorPool{ VK_NULL_HANDLE };
    
    void createDescriptorSets(void);
    std::vector<VkDescriptorSet> m_descriptorSets{};

    void createCommandBuffers(void);
    std::vector<VkCommandBuffer> m_cmdBuffers{};
    
    void recordCommandBuffer(uint32_t imageIndex);
    void transition_image_layout(
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkAccessFlags2 srcAccessMask,
        VkAccessFlags2 dstAccessMask,
        VkPipelineStageFlags2 srcStageMask,
        VkPipelineStageFlags2 dstStageMask,
        VkImageAspectFlags imageAspectFlags);

    void createSyncObjects();
    std::vector<VkSemaphore> m_graphicsSemaphores{};
    std::vector<VkSemaphore> m_presentSemaphores{};
    std::vector<VkFence> m_inFlightFences{};
    uint32_t m_currentFrame{ 0 };
    bool m_framebufferResized = false;
};
