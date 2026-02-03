#include <volk/volk.h>

#include <SDL3/SDL_video.h>

#include <string>
#include <vector>

constexpr int WIDTH = 1280;
constexpr int HEIGHT = 720;
constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char *> requiredDeviceExtension = {
	"VK_KHR_swapchain", "VK_KHR_shader_draw_parameters",
	"VK_KHR_synchronization2", "VK_KHR_dynamic_rendering"
};

const std::vector<const char *> validationLayers = { "VK_LAYER_KHRONOS_validation" };
#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

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
		createGraphicsPipeline();
		createCommandPool();
		createVertexBuffer();
		createIndexBuffer();
		createCommandBuffers();
		createSyncObjects();
	}

private:
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

	void createImageViews(void);
	std::vector<VkImageView> m_swapChainImageViews{};
	
	[[nodiscard]] std::vector<char> readFile(const std::string &filename) const; // helper function for createGraphicsPipeline()
	[[nodiscard]] VkShaderModule createShaderModule(const std::vector<char> &shaderCode) const; // helper function for createGraphicsPipeline()
	void createGraphicsPipeline(void);
	VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };
	VkPipeline m_graphicsPipeline{ VK_NULL_HANDLE };
	
	void createCommandPool(void);
	VkCommandPool m_cmdPool{ VK_NULL_HANDLE };

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

	void createCommandBuffers(void);
	std::vector<VkCommandBuffer> m_cmdBuffers{};

	void transition_image_layout(uint32_t imageIndex, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask, VkPipelineStageFlags2 srcStageMask, VkPipelineStageFlags2 dstStageMask);
	void recordCommandBuffer(const uint32_t imageIndex);
	
	void createSyncObjects(void);
	std::vector<VkSemaphore> m_graphicsSemaphores{};
	std::vector<VkSemaphore> m_presentSemaphores{};
	std::vector<VkFence> m_inFlightFences{};
	uint32_t m_currentFrame{ 0 };
	bool m_framebufferResized{ false };

	void drawFrame(void);

	void cleanup(void);
};
