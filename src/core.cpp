#define VOLK_IMPLEMENTATION
#include <volk.h>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_events.h>

#include "core.hpp"

#include <algorithm>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <vector>

void Core::createWindow(void)
{
	/* Initialize SDL's video subsystem - we need this for working with windows */
	if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
	{
		throw std::runtime_error("SDL_InitSubSystem() failed: " + std::string(SDL_GetError()));
	}

	/* Create a Vulkan-compatible SDL window */
	m_window = SDL_CreateWindow("VKVoxel", WIDTH, HEIGHT, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	if (!m_window)
	{
		throw std::runtime_error("SDL_CreateWindow() failed: " + std::string(SDL_GetError()));
	}
}

void Core::mainLoop(void)
{
	SDL_Event event;
	bool should_run = true;

	while (should_run)
	{
		/* Handle input */
		while (SDL_PollEvent(&event))
		{
			/* Exit the application if the user requested it, e.g. when the 'X' on the title bar is clicked, or Alt+F4 is pressed */
			if (event.type == SDL_EVENT_QUIT)
			{
				should_run = false;
				break;
			}

			/* Recreate the swap chain if the window is resized or minimized */
			if (event.window.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED || event.window.type == SDL_EVENT_WINDOW_MINIMIZED)
			{
				m_framebufferResized = true;
			}
		}

		/* Update state */

		/* Render frame */
		drawFrame();
		
		/* Wait for the device to finish what it's doing before proceeding */
		if (vkDeviceWaitIdle(m_device) != VK_SUCCESS)
		{
			throw std::runtime_error("vkDeviceWaitIdle() faile!");
		}
	}
}

void Core::loadVulkan(void)
{
	/* Initialize the Volk loader */
	if (volkInitialize() != VK_SUCCESS)
	{
		throw std::runtime_error("volkInitialize() failed!");
	}
}

void Core::createInstance(void)
{
	/* Define some information about our application, such as its name & version */
	const VkApplicationInfo appInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pNext = nullptr,
		.pApplicationName = "VKVoxel",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = VK_API_VERSION_1_4,
	};

	/* Enable validation layer if we're in debug build */
	std::vector<const char *> requiredLayers;
	if (enableValidationLayers)
	{
		requiredLayers.assign(validationLayers.begin(), validationLayers.end());
	}

	/* TODO: make sure we actually support these layers */

	/* Which extensions do we need for using SDL? */
	uint32_t sdlExtensionCount = 0;
	auto sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);
	if (sdlExtensions == nullptr)
	{
		throw std::runtime_error("SDL_Vulkan_GetInstanceExtensions() failed: " + std::string(SDL_GetError()));
	}

	/* TODO: make sure we actually support these extensions */

	/* Define some information about our instance, such as which layers and extensions we need */
	const VkInstanceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
		.ppEnabledLayerNames = requiredLayers.data(),
		.enabledExtensionCount = sdlExtensionCount,
		.ppEnabledExtensionNames = sdlExtensions,
	};

	/* Create the instance */
	if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
	{
		throw std::runtime_error("vkCreateInstance() failed!");
	}

	/* Load the instance */
	volkLoadInstance(m_instance);
}

void Core::createSurface(void)
{
	/* Create a Vulkan surface for our window */
	if (!SDL_Vulkan_CreateSurface(m_window, m_instance, nullptr, &m_surface))
	{
		throw std::runtime_error("SDL_Vulkan_CreateSurface() failed: " + std::string(SDL_GetError()));
	}
}

void Core::pickPhysicalDevice(void)
{
	/* Enumerate all Vulkan-compatible GPUs */
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);
	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data());

	if (physicalDevices.empty())
	{
		throw std::runtime_error("No Vulkan-compatible GPUs found!");
	}

	/* Iterate through all GPUs */
	for (const auto& physicalDevice : physicalDevices)
	{
		/* Get the GPU's properties*/
		VkPhysicalDeviceProperties2 properties{};
		properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		vkGetPhysicalDeviceProperties2(physicalDevice, &properties);

		/* Get the GPU's features */
		VkPhysicalDeviceFeatures2 features{};
		features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		vkGetPhysicalDeviceFeatures2(physicalDevice, &features);

		/* Get the GPU's supported extensions */
		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());

		/* Get the GPU's queue family properties */
		uint32_t queueFamilyPropertyCount = 0;
		VkQueueFamilyProperties2 queueFamilyProperties{};
		queueFamilyProperties.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
		vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, &queueFamilyPropertyCount, &queueFamilyProperties);
	
		/* Skip GPUs that don't support Vulkan 1.3 */
		if (properties.properties.apiVersion < VK_API_VERSION_1_4)
		{
			continue;
		}

		/* Skip GPUs that don't support graphic queue families */
		if (!!(queueFamilyProperties.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			continue;
		}

		/* TODO: make sure we actually support requiredDeviceExtension */
		/* TODO: make sure device extensions (such as dynamic rendering) are available? Though they should be mandatory for Vulkan 1.3 */


		/* Skip GPUs that don't support presentation to a surface */
		VkBool32 supportsSurface = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, 0, m_surface, &supportsSurface);
		if (supportsSurface != VK_TRUE)
		{
			continue;
		}

		m_physicalDevice = physicalDevice;
		break;
	}

	/* Double check to make sure a GPU was actually picked */
	if (m_physicalDevice == nullptr)
	{
		throw std::runtime_error("No Vulkan-compatible GPU was picked!");
	}
}

void Core::createLogicalDevice(void)
{
	/* Define some information about the queue(s) we want to create for our logical device */
	constexpr float queuePriorities = 1.0f;
	const VkDeviceQueueCreateInfo queueCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.queueFamilyIndex = 0, // TODO: find this
		.queueCount = 1,
		.pQueuePriorities = &queuePriorities,
	};

	/* Create a structure chain with the extensions we want to enable */
	const VkPhysicalDeviceFeatures2 deviceFeatures{};

	VkPhysicalDeviceSynchronization2Features synchronization2_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
		.pNext = nullptr,
		.synchronization2 = VK_TRUE,
	};

	VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES,
		.pNext = &synchronization2_features,
		.shaderDrawParameters = VK_TRUE,
	};

	VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_feature = {
    	.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
		.pNext = &shader_draw_parameters_features,
    	.dynamicRendering = VK_TRUE,
	};

	/* Create the logical device */
	const VkDeviceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &dynamic_rendering_feature,
		.flags = 0,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueCreateInfo,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtension.size()),
		.ppEnabledExtensionNames = requiredDeviceExtension.data(),
		.pEnabledFeatures = &deviceFeatures.features, // TODO: fill this out
	};

	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
	{
		throw std::runtime_error("vkCreateDevice() failed!");
	}

	/* Get the logical device's queue(s) */
	vkGetDeviceQueue(m_device, 0, 0, &m_graphicsQueue); // TODO: find queueFamilyIndex
	vkGetDeviceQueue(m_device, 0, 0, &m_presentQueue);  // TODO: find queueFamilyIndex

	/* Load the logical device */
	volkLoadDevice(m_device);
}

void Core::createSwapChain(void)
{
	/* Get swap chain capabilities & extent (number of images we can hold, and their resolution) */
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &m_swapChainCapabilities);
	
	if (m_swapChainCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		m_swapChainExtent = m_swapChainCapabilities.currentExtent;
	}
	else
	{
		int width = 0, height = 0;
		if (!SDL_GetWindowSizeInPixels(m_window, &width, &height))
		{
			throw std::runtime_error("SDL_GetWindowSizeInPixels() failed: " + std::string(SDL_GetError()));
		}
		
		m_swapChainExtent = {
			.width = std::clamp<uint32_t>(width, m_swapChainCapabilities.minImageExtent.width, m_swapChainCapabilities.maxImageExtent.width),
			.height = std::clamp<uint32_t>(height, m_swapChainCapabilities.minImageExtent.height, m_swapChainCapabilities.maxImageExtent.height)
		};
	}

	/* Get swap chain surface format (pixel format & color space) */
	uint32_t availableSurfaceFormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &availableSurfaceFormatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> availableSurfaceFormats(availableSurfaceFormatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &availableSurfaceFormatCount, availableSurfaceFormats.data());
	
	for (const auto &availableSurfaceFormat : availableSurfaceFormats)
	{
		if (availableSurfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableSurfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			m_swapChainSurfaceFormat = availableSurfaceFormat;
			break;
		}

		m_swapChainSurfaceFormat = availableSurfaceFormat;
	}

	m_swapChainImageFormat = m_swapChainSurfaceFormat.format;

	/* Get swap chain present mode (triple-buffering, vsync, primitive) */
	uint32_t availablePresentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &availablePresentModeCount, nullptr);
	std::vector<VkPresentModeKHR> availablePresentModes{};
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &availablePresentModeCount, availablePresentModes.data());

	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			m_swapChainPresentMode = availablePresentMode;
			break;
		}

		m_swapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	}

	/* Get swap chain image count */
	uint32_t imageCount = m_swapChainCapabilities.minImageCount + 1;
	if (m_swapChainCapabilities.maxImageCount > 0 && imageCount > m_swapChainCapabilities.maxImageCount)
	{
		imageCount = m_swapChainCapabilities.maxImageCount;
	}

	/* Define some information about our swap chain */
	//const std::array<uint32_t, 2> queueFamilyIndices = { 0, 1 };
	const VkSwapchainCreateInfoKHR createInfo = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = nullptr,
		.flags = 0,
		.surface = m_surface,
		.minImageCount = imageCount,
		.imageFormat = m_swapChainSurfaceFormat.format,
		.imageColorSpace = m_swapChainSurfaceFormat.colorSpace,
		.imageExtent = m_swapChainExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE, // or VK_SHARING_MODE_CONCURRENT, depending on if graphicsFamily == presentFamily
		.queueFamilyIndexCount = 0, // 2 if imageSharingMode is CONCURRENT, 0 if EXCLUSIVE
		.pQueueFamilyIndices = nullptr, // queueFamilyIndices if CONCURRENT, nullptr if EXCLUSIVE
		.preTransform = m_swapChainCapabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = m_swapChainPresentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = nullptr,
	};

	/* Create swap chain */
	if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("vkCreateSwapchainKHR() failed!");
	}

	/* Get presentable images associated with our swap chain */
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
	m_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());
}

void Core::cleanupSwapChain(void)
{
	for (auto imageView : m_swapChainImageViews)
	{
		vkDestroyImageView(m_device, imageView, nullptr);
		imageView = nullptr;
	}

	vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
	m_swapChain = nullptr;
}

void Core::recreateSwapChain(void)
{
	if (vkDeviceWaitIdle(m_device) != VK_SUCCESS)
	{
		throw std::runtime_error("vkDeviceWaitIdle() failed!");
	}

	cleanupSwapChain();

	createSwapChain();
    createImageViews();
}

void Core::createImageViews(void)
{
	m_swapChainImageViews.clear();
	m_swapChainImageViews.resize(m_swapChainImages.size());

	for (size_t i = 0; i < m_swapChainImageViews.size(); i++)
	{
		const VkImageViewCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = m_swapChainImages.at(i),
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = m_swapChainImageFormat,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};

		if (vkCreateImageView(m_device, &createInfo, nullptr, &m_swapChainImageViews.at(i)) != VK_SUCCESS)
		{
			throw std::runtime_error("vkCreateImageView() failed!");
		}
	}
}

[[nodiscard]] std::vector<char> Core::readFile(const std::string &filename) const
{
	/* Open file */
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file " + std::string(filename));
	}

	/* Create a buffer to hold the contents of the file */
	const size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	/* Seek back to the beginning of the file and read all data */
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	/* Clean up */
	file.close();

	return buffer;
}

[[nodiscard]] VkShaderModule Core::createShaderModule(const std::vector<char> &shaderCode) const
{
	VkShaderModule shaderModule;

	const VkShaderModuleCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.codeSize = shaderCode.size(),
		.pCode = reinterpret_cast<const uint32_t *>(shaderCode.data()),
	};

	if (vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
    	throw std::runtime_error("vkCreateShaderModule() failed!");
	}

	return shaderModule;
}

void Core::createGraphicsPipeline(void)
{
	/* Dynamic state - we can define these properties dynamically at draw time rather than bake it into our immutable pipeline */
	const std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	const VkPipelineDynamicStateCreateInfo dynamicState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data(),
	};

	/* Viewports and scissors */
	const VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(m_swapChainExtent.width),
		.height = static_cast<float>(m_swapChainExtent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	const VkRect2D scissor = {
		.offset = VkOffset2D{ .x = 0, .y = 0 },
		.extent = m_swapChainExtent,
	};

	const VkPipelineViewportStateCreateInfo viewportCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};

	/* Vertex input - TODO: change this when vertices are no longer hardcoded in the vertex shader */
	const VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.vertexBindingDescriptionCount = 0,
		.pVertexBindingDescriptions = nullptr,
		.vertexAttributeDescriptionCount = 0,
		.pVertexAttributeDescriptions = nullptr,
	};

	/* Input assembly */
	const VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE,
	};

	/* Vertex shader*/
	const VkShaderModule vertShaderModule = createShaderModule(readFile("shaders/shader.slang.spv"));

	const VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vertShaderModule,
		.pName = "vertMain",
		.pSpecializationInfo = nullptr,
	};

	/* Rasterizer */
	const VkPipelineRasterizationStateCreateInfo rasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE, // VK_FRONT_FACE_COUNTER_CLOCKWISE?
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.0f,
		.depthBiasClamp = 0.0f,
		.depthBiasSlopeFactor = 1.0f,
		.lineWidth = 1.0f,
	};

	/* Multisampling */
	const VkPipelineMultisampleStateCreateInfo multisampler = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1.0f,
		.pSampleMask = nullptr,
		.alphaToCoverageEnable = VK_FALSE,
		.alphaToOneEnable = VK_FALSE,
	};

	/* Depth and stencil testing
	const VkPipelineDepthStencilStateCreateInfo depthStencil = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.depthTestEnable = 0,
		.depthWriteEnable = 0,
		.depthCompareOp = 0,
		.depthBoundsTestEnable = 0,
		.stencilTestEnable = 0,
		.front = 0,
		.back = 0,
		.minDepthBounds = 0,
		.maxDepthBounds = 0,
	};
	*/

	/* Color blending */
	const VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.blendEnable = VK_FALSE, // VK_TRUE
		.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, // VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
		.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	};

	const VkPipelineColorBlendStateCreateInfo colorBlending = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
		.blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
	};

	/* Fragment shader */
    const VkShaderModule fragShaderModule = createShaderModule(readFile("shaders/shader.slang.spv"));

	const VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fragShaderModule,
		.pName = "fragMain",
		.pSpecializationInfo = nullptr,
	};

	/* Shader stages */
	const std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

	/* Pipeline layout */
	const VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.setLayoutCount = 0,
		.pSetLayouts = nullptr,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = nullptr,
	};

	if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
	{
    	throw std::runtime_error("vkCreatePipelineLayout() failed!");
	}

	/* Create pipeline */
	const VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.pNext = nullptr,
		.viewMask = 0,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &m_swapChainImageFormat,
		.depthAttachmentFormat = VK_FORMAT_UNDEFINED,
		.stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
	};
	
	const VkGraphicsPipelineCreateInfo pipelineInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &pipelineRenderingCreateInfo,
		.flags = 0,
		.stageCount = static_cast<uint32_t>(shaderStages.size()),
		.pStages = shaderStages.data(),
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssemblyInfo,
		.pTessellationState = nullptr,
		.pViewportState = &viewportCreateInfo,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampler,
		.pDepthStencilState = nullptr,
		.pColorBlendState = &colorBlending,
		.pDynamicState = &dynamicState,
		.layout = m_pipelineLayout,
		.renderPass = nullptr,
		.subpass = 0,
		.basePipelineHandle = nullptr,
		.basePipelineIndex = -1,
	};

	if (vkCreateGraphicsPipelines(m_device, nullptr, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("VkGraphicsPipelineCreateInfo() failed!");
	}

	/* Clean up */
	vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
}

void Core::createCommandPool(void)
{
	const VkCommandPoolCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = 0, // TODO: use graphics family queue index
	};

	if (vkCreateCommandPool(m_device, &createInfo, nullptr, &m_cmdPool) != VK_SUCCESS)
	{
		throw std::runtime_error("vkCreateCommandPool() failed!");
	}
}

void Core::createCommandBuffers(void)
{
	m_cmdBuffers.clear();
	m_cmdBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	const VkCommandBufferAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = m_cmdPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = static_cast<uint32_t>(m_cmdBuffers.size()),
	};

	if (vkAllocateCommandBuffers(m_device, &allocInfo, m_cmdBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("vkAllocateCommandBuffers() failed!");
	}
}

void Core::transition_image_layout
(
    uint32_t imageIndex,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	VkAccessFlags2 srcAccessMask,
	VkAccessFlags2 dstAccessMask,
	VkPipelineStageFlags2 srcStageMask,
	VkPipelineStageFlags2 dstStageMask
)
{
	const VkImageMemoryBarrier2 barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
		.pNext = nullptr,
        .srcStageMask = srcStageMask,
        .srcAccessMask = srcAccessMask,
        .dstStageMask = dstStageMask,
        .dstAccessMask = dstAccessMask,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = m_swapChainImages.at(imageIndex),
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };

	const VkDependencyInfo dependencyInfo = {
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
		.pNext = nullptr,
        .dependencyFlags = {},
		.memoryBarrierCount = 0,
		.pMemoryBarriers = nullptr,
		.bufferMemoryBarrierCount = 0,
		.pBufferMemoryBarriers = nullptr,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier,
	};

	vkCmdPipelineBarrier2(m_cmdBuffers.at(m_currentFrame), &dependencyInfo);
}

void Core::recordCommandBuffer(uint32_t imageIndex)
{
	auto &cmdBuffer = m_cmdBuffers.at(m_currentFrame);

	/* Begin command buffer */
	const VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.pNext = nullptr,
		.flags = 0,
		.pInheritanceInfo = nullptr,
	};

	vkBeginCommandBuffer(cmdBuffer, &beginInfo);

	/* Transition the swap chain image to COLOR_ATTACHMENT_OPTIMAL before rendering */
	transition_image_layout(
    	imageIndex,
    	VK_IMAGE_LAYOUT_UNDEFINED,
    	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    	VK_ACCESS_NONE,
    	VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
    	VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    	VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
	);

	/* Set up the color attachment */
	const VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	
	const VkRenderingAttachmentInfo colorAttachment = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.pNext = nullptr,
		.imageView = m_swapChainImageViews.at(imageIndex),
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.resolveMode = VK_RESOLVE_MODE_NONE,
		.resolveImageView = nullptr,
		.resolveImageLayout = {},
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = clearColor,
	};

	/* Rendering info */
	const VkRenderingInfo renderingInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.pNext = nullptr,
		.flags = 0,
		.renderArea = {
			.offset = { .x = 0, .y = 0 },
			.extent = m_swapChainExtent
		},
		.layerCount = 1,
		.viewMask = 0,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachment,
		.pDepthAttachment = nullptr,
		.pStencilAttachment = nullptr,
	};

	/* Begin rendering */
	vkCmdBeginRendering(cmdBuffer, &renderingInfo);

	/* Bind our pipeline plus our dynamic state */
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

	const VkViewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(m_swapChainExtent.width),
		.height = static_cast<float>(m_swapChainExtent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

	const VkRect2D scissor = {
		.offset = { .x = 0, .y = 0 },
		.extent = m_swapChainExtent,
	};

	vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

	vkCmdDraw(cmdBuffer, 3, 1, 0 ,0);
	
	/* End rendering */
	vkCmdEndRendering(cmdBuffer);

	/* Transition the swap chain image to PRESENT_SRC after rendering */
	transition_image_layout(
    	imageIndex,
    	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    	VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    	VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
    	VK_ACCESS_NONE,
    	VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    	VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT
	);

	/* End command buffer */
	vkEndCommandBuffer(cmdBuffer);
}

void Core::createSyncObjects(void)
{
	m_graphicsSemaphores.clear();
	m_presentSemaphores.clear();
	m_inFlightFences.clear();

	m_graphicsSemaphores.resize(m_swapChainImages.size());
	m_presentSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < m_swapChainImages.size(); i++)
	{
		/* Create semaphores for our graphics queue */
		constexpr VkSemaphoreCreateInfo semaphoreCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
		};

		if (vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_graphicsSemaphores.at(i)) != VK_SUCCESS)
		{
			throw std::runtime_error("vkCreateSemaphore() failed!");
		}
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		/* Create semaphores for our presentation queue */
		constexpr VkSemaphoreCreateInfo semaphoreCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
		};

		if (vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_presentSemaphores.at(i)) != VK_SUCCESS)
		{
			throw std::runtime_error("vkCreateSemaphore() failed!");
		}

		/* Create fences for our frames in flight */
		constexpr VkFenceCreateInfo fenceCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};

		if (vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_inFlightFences.at(i)) != VK_SUCCESS)
		{
			throw std::runtime_error("vkCreateFence() failed!");
		}
	}
}

void Core::drawFrame(void)
{
	/* Wait for the previous frame to finish */
	if (vkWaitForFences(m_device, 1, &m_inFlightFences.at(m_currentFrame), VK_TRUE, UINT64_MAX) != VK_SUCCESS)
	{
		throw std::runtime_error("vkWaitForFences() failed!");
	}

	/* Acquire an image from the swap chain */
	uint32_t imageIndex = 0;

	VkResult result = vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_presentSemaphores.at(m_currentFrame), VK_NULL_HANDLE, &imageIndex);
	
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
    	recreateSwapChain();
    	return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
	    throw std::runtime_error("vkAcquireNextImageKHR() failed!");
	}

	/* Reset fence after acquiring image to prevent deadlock */
	if (vkResetFences(m_device, 1, &m_inFlightFences.at(m_currentFrame)) != VK_SUCCESS)
	{
		throw std::runtime_error("vkResetFences() failed!");
	}

	/* Record a command buffer which draws the scene onto that image */
	vkResetCommandBuffer(m_cmdBuffers.at(m_currentFrame), 0);
	recordCommandBuffer(imageIndex);

	/* Submit the recorded command buffer */
	const VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	const VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_presentSemaphores.at(m_currentFrame),
		.pWaitDstStageMask = &waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &m_cmdBuffers.at(m_currentFrame),
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &m_graphicsSemaphores.at(m_currentFrame),
	};

	if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences.at(m_currentFrame)) != VK_SUCCESS)
	{
		throw std::runtime_error("vkQueueSubmit() failed!");
	}

	/* Present the swap chain image */
	const VkPresentInfoKHR presentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_graphicsSemaphores.at(m_currentFrame),
		.swapchainCount = 1,
		.pSwapchains = &m_swapChain,
		.pImageIndices = &imageIndex,
		.pResults = nullptr,
	};

	/*VkResult*/ result = vkQueuePresentKHR(m_presentQueue, &presentInfo);


	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
	{
		m_framebufferResized = false;
    	recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
    	throw std::runtime_error("vkQueuePresentKHR() failed!");
	}

	/* Move the current frame index */
	m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Core::cleanup(void)
{
	/* Clean up Vulkan stuff */
	for (size_t i = 0; i < m_swapChainImages.size(); i++)
	{
		vkDestroySemaphore(m_device, m_graphicsSemaphores.at(i), nullptr);
		m_graphicsSemaphores.at(i) = nullptr;
	}

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(m_device, m_presentSemaphores.at(i), nullptr);
		m_presentSemaphores.at(i) = nullptr;

		vkDestroyFence(m_device, m_inFlightFences.at(i), nullptr);
		m_inFlightFences.at(i) = nullptr;
	}

	vkDestroyCommandPool(m_device, m_cmdPool, nullptr);
	m_cmdPool = nullptr;

	vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
	m_graphicsPipeline = nullptr;

	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
	m_pipelineLayout = nullptr;

	cleanupSwapChain();

	vkDestroyDevice(m_device, nullptr);
	m_device = nullptr;

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	m_surface = nullptr;

	vkDestroyInstance(m_instance, nullptr);
	m_instance = nullptr;

	/* Clean up SDL stuff */
	SDL_DestroyWindow(m_window);
	m_window = nullptr;
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_Quit();
}
