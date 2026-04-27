#include <SDL3/SDL_events.h>
#include <volk/volk.h>

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_timer.h>

#include <string>
#include <vector>

constexpr int WIDTH = 1280;
constexpr int HEIGHT = 720;
constexpr uint8_t MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char *> requiredDeviceExtension = {
	"VK_KHR_swapchain", "VK_KHR_shader_draw_parameters",
	"VK_KHR_synchronization2", "VK_KHR_dynamic_rendering",
	"VK_KHR_index_type_uint8"
};

const std::vector<const char *> validationLayers = { "VK_LAYER_KHRONOS_validation" };
#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

#include "voxel.hpp"

const std::string MODEL_PATH = "resources/models/voxel.obj";
const std::string TEXTURE_PATH = "resources/textures/grass_block_side.jpg";

class Core
{
public:
	void run(void)
	{
		createWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	void initVulkan(void)
	{
		loadVulkan();
		createInstance();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createCommandPool();
		createDepthResources();
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		loadModel();
		createVertexBuffer();
		createIndexBuffer();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
		createSyncObjects();
	}

private:
	glm::vec3 camPos { 0.0f, 0.0f, -6.0f };
	glm::vec3 objectRotations[1]{};
	uint64_t lastTime = SDL_GetTicks();

	SDL_Window* m_window{ nullptr };
	void createWindow(void);

	void mainLoop(void);

	void loadVulkan(void);

	void createInstance(void);
	VkInstance m_instance{ VK_NULL_HANDLE };
	
	void createSurface(void);
	VkSurfaceKHR m_surface{ VK_NULL_HANDLE };

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
	VkDescriptorSetLayout m_descriptorSetLayout;
	
	[[nodiscard]] std::vector<char> readFile(const std::string &filename) const; // helper function for createGraphicsPipeline()
	[[nodiscard]] VkShaderModule createShaderModule(const std::vector<char> &shaderCode) const; // helper function for createGraphicsPipeline()
	void createGraphicsPipeline(void);
	VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };
	VkPipeline m_graphicsPipeline{ VK_NULL_HANDLE };
	
	void createCommandPool(void);
	VkCommandPool m_cmdPool{ VK_NULL_HANDLE };

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat findDepthFormat(void);
	bool hasStencilComponent(const VkFormat &format);
	void createDepthResources(void);
	VkImage m_depthImage{ VK_NULL_HANDLE };
	VkDeviceMemory m_depthImageMemory{ VK_NULL_HANDLE };
	VkImageView m_depthImageView{ VK_NULL_HANDLE };

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);
	void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void createTextureImage(void);
	VkCommandBuffer beginSingleTimeCommands(void);
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	void copyBuffer1(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkDeviceSize size);
	VkImage m_textureImage { VK_NULL_HANDLE };
	VkDeviceMemory m_textureImageMemory { VK_NULL_HANDLE };

	void createTextureImageView(void);
	VkImageView m_textureImageView{ VK_NULL_HANDLE };

	void createTextureSampler(void);
	VkSampler m_textureSampler{ VK_NULL_HANDLE };

	void loadModel(void);
	std::vector<Voxel> m_vertices;
	std::vector<uint16_t> m_indices;

	// helper function for CreateVertexBuffer() and createIndexBuffer()
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkDeviceSize &size);

	void createVertexBuffer(void);
	VkBuffer m_vertexBuffer{ VK_NULL_HANDLE };
	VkBuffer m_indexBuffer{ VK_NULL_HANDLE };

	void createIndexBuffer(void);
	VkDeviceMemory m_vertexBufferMemory{ VK_NULL_HANDLE };
	VkDeviceMemory m_indexBufferMemory{ VK_NULL_HANDLE };

	void createUniformBuffers(void);
	void updateUniformBuffer(const uint32_t currentFrame);
	std::vector<VkBuffer> m_uniformBuffers;
	std::vector<VkDeviceMemory> m_uniformBuffersMemory;
	std::vector<void*> m_uniformBuffersMapped;

	void createDescriptorPool(void);
	VkDescriptorPool m_descriptorPool{ VK_NULL_HANDLE };

	void createDescriptorSets(void);
	std::vector<VkDescriptorSet> m_descriptorSets{};

	void createCommandBuffers(void);
	std::vector<VkCommandBuffer> m_cmdBuffers{};

	void transition_image_layout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask, VkPipelineStageFlags2 srcStageMask, VkPipelineStageFlags2 dstStageMask, VkImageAspectFlags image_aspect_flags);
	void recordCommandBuffer(const uint32_t imageIndex);
	
	void createSyncObjects(void);
	std::vector<VkSemaphore> m_graphicsSemaphores{};
	std::vector<VkSemaphore> m_presentSemaphores{};
	std::vector<VkFence> m_inFlightFences{};
	uint32_t m_currentFrame{ 0 };
	bool m_framebufferResized{ false };

	void processInput(const SDL_Event &event);

	void drawFrame(void);

	void cleanup(void);
};
