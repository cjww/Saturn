#include "pch.h"
#include "VulkanCore.hpp"

#include "debugFunctions.h" // for checkError and debugCallback

namespace sa {
	uint32_t VulkanCore::getQueueFamilyIndex(vk::QueueFlags capabilities, vk::QueueFamilyProperties* prop) {
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();

		uint32_t queueFamilyIndex = 0;
		for (auto properties : queueFamilyProperties) {
			if (properties.queueCount > 0) {
				if (properties.queueFlags & capabilities) {
					if (prop != nullptr) {
						*prop = properties;
					}
					return queueFamilyIndex;
				}
			}
			queueFamilyIndex++;
		}
		return UINT32_MAX;
	}

	QueueInfo VulkanCore::getQueueInfo(vk::QueueFlags capabilities, uint32_t maxCount) {
		vk::QueueFamilyProperties prop;
		QueueInfo queueInfo = {};
		queueInfo.family = getQueueFamilyIndex(capabilities, &prop);
		if (queueInfo.family == UINT32_MAX) {
			throw std::runtime_error("Found no suitable queue family");
		}

		queueInfo.queueCount = std::min(maxCount, prop.queueCount);

		queueInfo.priorities.resize(queueInfo.queueCount);
		for (uint32_t i = 0; i < queueInfo.queueCount; i++) {
			queueInfo.priorities[i] = 1.0f;
		}

		return queueInfo;
	}
	void VulkanCore::setupDebug() {

		m_validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		std::vector<vk::LayerProperties> layerProperties = vk::enumerateInstanceLayerProperties();

		for (int i = 0; i < static_cast<int>(m_validationLayers.size()); i++) {
			bool found = false;
			for (const auto& prop : layerProperties) {
				if (strcmp(m_validationLayers[i], prop.layerName) == 0) {
					found = true;
					break;
				}
			}
			if (!found) {
				DEBUG_LOG_WARNING("A validation layer requested was not present in available layers: ", m_validationLayers[i]);
				m_validationLayers.erase(m_validationLayers.begin() + i);
				i--;
			}
		}
	}

	void VulkanCore::createInstance() {
		vk::ApplicationInfo appInfo{
			.pApplicationName = "Saturn App",
			.applicationVersion = 0,
			.pEngineName = "Saturn",
			.engineVersion = 0,
			.apiVersion = VK_API_VERSION_1_3
		};

		uint32_t count = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
		for (uint32_t i = 0; i < count; i++) {
			m_instanceExtensions.push_back(glfwExtensions[i]);
		}
		m_instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		m_instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

		vk::InstanceCreateInfo instanceInfo{
			.pApplicationInfo = &appInfo,
			.enabledExtensionCount = static_cast<uint32_t>(m_instanceExtensions.size()),
			.ppEnabledExtensionNames = m_instanceExtensions.data()
		};

		std::cout << "---Extensions---" << std::endl;
		for (const char* extension : m_instanceExtensions) {
			std::cout << extension << std::endl;
		}

		if (!m_validationLayers.empty()) {
			instanceInfo.ppEnabledLayerNames = m_validationLayers.data();
			instanceInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());

			std::cout << "---Layers---" << std::endl;
			for (uint32_t i = 0; i < m_validationLayers.size(); i++) {
				std::cout << m_validationLayers[i] << std::endl;
			}


			vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{
				.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
				.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
				.pfnUserCallback = sa::debugCallback
			};


			instanceInfo.pNext = &debugMessengerCreateInfo;
		}
		sa::checkError(
			vk::createInstance(&instanceInfo, nullptr, &m_instance),
			"Failed to create instance"
		);
	}

	void VulkanCore::findPhysicalDevice() {
		
		std::vector<vk::PhysicalDevice> physicalDevices = m_instance.enumeratePhysicalDevices();
		
		// Rank Physical Device
		int highestScore = 0;
		for (vk::PhysicalDevice device : physicalDevices) {
			vk::PhysicalDeviceProperties properties = device.getProperties();
			vk::PhysicalDeviceFeatures features = device.getFeatures();

			int score = 0;
			score += properties.limits.maxColorAttachments;
			
			if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
				score += 1000;
			}

			if (!features.geometryShader) {
				score = 0;
			}
			if (score > highestScore) {
				highestScore = score;
				m_physicalDevice = device;
			}
		}
		if (!m_physicalDevice) {
			throw std::runtime_error("No GPU Found");
		}
	}

	void VulkanCore::createDevice() {
		m_deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		m_graphicsQueueInfo = getQueueInfo(vk::QueueFlagBits::eGraphics, 1);
		m_computeQueueInfo = getQueueInfo(vk::QueueFlagBits::eCompute, 1);

		std::vector<QueueInfo> queueInfos;
		if (m_graphicsQueueInfo.family == m_computeQueueInfo.family) {
			m_graphicsQueueInfo = getQueueInfo(vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute, 2);
			queueInfos.push_back(m_graphicsQueueInfo);
		}
		else {
			queueInfos.push_back(m_graphicsQueueInfo);
			queueInfos.push_back(m_computeQueueInfo);
		}

		vk::PhysicalDeviceFeatures features = m_physicalDevice.getFeatures();


		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
		for (const auto& queueInfo : queueInfos) {
			queueCreateInfos.push_back(vk::DeviceQueueCreateInfo{
				.queueFamilyIndex = queueInfo.family,
				.queueCount = queueInfo.queueCount,
				.pQueuePriorities = queueInfo.priorities.data()
			});
		}

		vk::DeviceCreateInfo deviceInfo{
			.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
			.pQueueCreateInfos = queueCreateInfos.data(),
			.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size()),
			.ppEnabledExtensionNames = m_deviceExtensions.data(),
			.pEnabledFeatures = &features
		};

		if (!m_validationLayers.empty()) {
			deviceInfo.setPpEnabledLayerNames(m_validationLayers.data());
			deviceInfo.setEnabledLayerCount(m_validationLayers.size());
		}


		m_device = m_physicalDevice.createDevice(deviceInfo);

		m_graphicsQueue = m_device.getQueue(m_graphicsQueueInfo.family, 0);

		if (m_graphicsQueueInfo.family != m_computeQueueInfo.family) {
			m_computeQueue = m_device.getQueue(m_computeQueueInfo.family, 0);
		}
		else {
			m_computeQueue = m_graphicsQueue;
		}
	}

	void VulkanCore::createCommandPools() {
		m_graphicsCommandPool.create(m_device, m_graphicsQueueInfo.family, vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
		m_computeCommandPool.create(m_device, m_computeQueueInfo.family, vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
	}

	bool VulkanCore::isDepthFormat(vk::Format format) {
		return format == vk::Format::eD16Unorm
			|| format == vk::Format::eD16UnormS8Uint
			|| format == vk::Format::eD24UnormS8Uint
			|| format == vk::Format::eD32Sfloat
			|| format == vk::Format::eD32SfloatS8Uint;
	}

	bool VulkanCore::isColorFormat(vk::Format format) {
		return !isDepthFormat(format);
	}

	void VulkanCore::init() {
#if RENDERER_VALIDATION
			setupDebug();
#endif // RENDERER_VALIDATION
		createInstance();
		findPhysicalDevice();
		createDevice();
		
		createCommandPools();


		m_defaultColorFormat = vk::Format::eR8G8B8A8Srgb;
		m_defaultDepthFormat = vk::Format::eD16Unorm;
	}

	void VulkanCore::cleanup() {
		

		m_graphicsCommandPool.destroy();
		m_computeCommandPool.destroy();

		m_device.destroy();
		m_instance.destroy();
	}

	vk::SurfaceKHR VulkanCore::createSurface(GLFWwindow* pWindow) {
		VkSurfaceKHR surface;
		checkError(
			(vk::Result)glfwCreateWindowSurface(m_instance, pWindow, nullptr, &surface),
			"Failed to create window surface",
			true
		);
		return surface;
	}

	vk::SwapchainKHR VulkanCore::createSwapchain(vk::SurfaceKHR surface, uint32_t queueFamily, vk::Format* outFormat) {
		if (!m_physicalDevice.getSurfaceSupportKHR(queueFamily, surface)) {
			throw std::runtime_error("Surface unsupported by this Device");
		}

		vk::SurfaceCapabilitiesKHR surfaceCapabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(surface);

		// Calculate optimal image count
		uint32_t minImageCount = surfaceCapabilities.minImageCount + 1;
		if (surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount) {
			minImageCount = surfaceCapabilities.maxImageCount;
		}

		// Find fitting surface format
		std::vector<vk::SurfaceFormatKHR> formats = m_physicalDevice.getSurfaceFormatsKHR(surface);
		vk::SurfaceFormatKHR finalFormat = formats.at(0);
		for (auto format : formats) {
			vk::FormatProperties properties = m_physicalDevice.getFormatProperties(format.format);
			if ((properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachment) == vk::FormatFeatureFlagBits::eColorAttachment) {
				finalFormat = format;
				break;
			}
		}

		// find Mailbox present mode
		vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo; // Assumes this present mode is always present
		std::vector<vk::PresentModeKHR> allPresentModes = m_physicalDevice.getSurfacePresentModesKHR(surface);
		for (auto mode : allPresentModes) {
			if (mode == vk::PresentModeKHR::eMailbox) {
				presentMode = mode;
				break;
			}
		}

		vk::SwapchainCreateInfoKHR info{
			.surface = surface,
			.minImageCount = minImageCount,
			.imageFormat = finalFormat.format,
			.imageColorSpace = finalFormat.colorSpace,
			.imageExtent = surfaceCapabilities.currentExtent,
			.imageArrayLayers = 1,
			.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
			.imageSharingMode = vk::SharingMode::eExclusive,
			.queueFamilyIndexCount = 1,
			.pQueueFamilyIndices = &queueFamily,
			.preTransform = surfaceCapabilities.currentTransform,
			.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode = presentMode,
			.clipped = true,
		};

		if(outFormat) *outFormat = finalFormat.format;

		return m_device.createSwapchainKHR(info);
	}

	vk::ImageView VulkanCore::createImageView(vk::ImageViewType type, vk::Image image, vk::Format format, vk::ImageAspectFlags aspectMask, uint32_t baseMipLevel, uint32_t baseArrayLevel) {
		vk::ImageViewCreateInfo info{
			.image = image,
			.viewType = type,
			.format = format,
			.components = {
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity
			},			
			.subresourceRange = {
				.aspectMask = aspectMask,
				.baseMipLevel = baseMipLevel,
				.levelCount = 1,
				.baseArrayLayer = baseArrayLevel,
				.layerCount = 1
			}
		};

		return m_device.createImageView(info);
	}

	vk::Framebuffer VulkanCore::createFrameBuffer(vk::RenderPass renderPass, std::vector<vk::ImageView> attachments, uint32_t width, uint32_t height, uint32_t layers){
		vk::FramebufferCreateInfo info{
			.renderPass = renderPass,
			.width = width,
			.height = height,
			.layers = layers
		};
		info.setAttachments(attachments);

		return m_device.createFramebuffer(info);
	}

	vk::Pipeline VulkanCore::createGraphicsPipeline(vk::PipelineLayout layout, vk::RenderPass renderPass, uint32_t subpassIndex, vk::Extent2D extent, std::vector<vk::PipelineShaderStageCreateInfo> shaderStages, vk::PipelineVertexInputStateCreateInfo vertexInput, vk::PipelineCache cache,  PipelineConfig config) {

		vk::PipelineInputAssemblyStateCreateInfo input{
			.topology = config.input.topology,
			.primitiveRestartEnable = false,
		};

		vk::PipelineRasterizationStateCreateInfo rasterizer = {
			.depthClampEnable = false,
			.rasterizerDiscardEnable = false, //TODO: look into what this is
			.polygonMode = config.rasterizer.polygonMode,
			.cullMode = config.rasterizer.cullMode,
			.frontFace = config.rasterizer.frontFace,
			.depthBiasEnable = false,
			.lineWidth = 1.0f,
		};

		vk::PipelineMultisampleStateCreateInfo multisample{
			.rasterizationSamples = config.multisample.sampleCount,
			.sampleShadingEnable = false,
		};

		std::vector<vk::PipelineColorBlendAttachmentState> colorAttachments(config.colorBlends.size());
		for (int i = 0; i < config.colorBlends.size(); i++) {

			colorAttachments[i].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
			colorAttachments[i].blendEnable = config.colorBlends[i].enable;
			colorAttachments[i].colorBlendOp = config.colorBlends[i].colorBlendOp;
			colorAttachments[i].alphaBlendOp = config.colorBlends[i].alphaBlendOp;
			colorAttachments[i].srcColorBlendFactor = config.colorBlends[i].srcColorBlendFactor;
			colorAttachments[i].dstColorBlendFactor = config.colorBlends[i].dstColorBlendFactor;
			colorAttachments[i].srcAlphaBlendFactor = config.colorBlends[i].srcAlphaBlendFactor;
			colorAttachments[i].dstAlphaBlendFactor = config.colorBlends[i].dstAlphaBlendFactor;
		}

		vk::PipelineColorBlendStateCreateInfo colorBlend{
			.logicOpEnable = false,
			.logicOp = vk::LogicOp::eAnd
		};
		colorBlend.setAttachments(colorAttachments);


		vk::PipelineDepthStencilStateCreateInfo depthState = {
			.depthTestEnable = config.depthStencil.depthTestEnable,
			.depthWriteEnable = config.depthStencil.depthWriteEnable,
			.depthCompareOp = config.depthStencil.depthCompareOp,
			.depthBoundsTestEnable = config.depthStencil.depthBoundsTestEnable,
			.stencilTestEnable = config.depthStencil.stencilTestEnable,
		};


		vk::Viewport viewport = {
			.x = 0.0f,
			.y = 0.0f,
			.width = (float)extent.width,
			.height = (float)extent.height,
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};

		vk::Rect2D scissor = {
			.offset = { 0, 0 },
			.extent = extent,
		};

		vk::PipelineViewportStateCreateInfo viewportState = {
			.viewportCount = 1,
			.pViewports = &viewport,
			.scissorCount = 1,
			.pScissors = &scissor,
		};

		vk::PipelineDynamicStateCreateInfo dynamicState;
		dynamicState.setDynamicStates(config.dynamicStates);

		vk::GraphicsPipelineCreateInfo info{
			.pVertexInputState = &vertexInput,
			.pInputAssemblyState = &input,
			.pTessellationState = nullptr,
			.pViewportState = &viewportState,
			.pRasterizationState = &rasterizer,
			.pMultisampleState = &multisample,
			.pDepthStencilState = &depthState,
			.pColorBlendState = &colorBlend,
			.pDynamicState = &dynamicState,
			.layout = layout,
			.renderPass = renderPass,
			.subpass = subpassIndex,
		};
		info.setStages(shaderStages);

		auto result = m_device.createGraphicsPipeline(cache, info);
		checkError(result.result, "Failed to create Graphics pipeline", true);
		return result.value;
	}


	uint32_t VulkanCore::getGraphicsQueueFamily() const {
		return m_graphicsQueueInfo.family;
	}

	vk::Queue VulkanCore::getGraphicsQueue() const {
		return m_graphicsQueue;
	}

	uint32_t VulkanCore::getComputeQueueFamily() const {
		return m_computeQueueInfo.family;
	}

	vk::Queue VulkanCore::getComputeQueue() const {
		return m_computeQueue;
	}

	vk::Instance VulkanCore::getInstance() const {
		return m_instance;
	}

	vk::PhysicalDevice VulkanCore::getPhysicalDevice() const {
		return m_physicalDevice;
	}

	vk::Device VulkanCore::getDevice() const {
		return m_device;
	}

	vk::Format VulkanCore::getDefaultColorFormat() const {
		return m_defaultColorFormat;
	}

	vk::Format VulkanCore::getDefaultDepthFormat() const {
		return m_defaultDepthFormat;
	}

	CommandBufferSet VulkanCore::allocateGraphicsCommandBufferSet(uint32_t count, vk::CommandBufferLevel level) {
		return m_graphicsCommandPool.allocateCommandBufferSet(count, level);
	}

	CommandBufferSet VulkanCore::allocateComputeCommandBufferSet(uint32_t count, vk::CommandBufferLevel level) {
		return m_computeCommandPool.allocateCommandBufferSet(count, level);
	}

	FramebufferSet VulkanCore::createFrameBufferSet(vk::RenderPass renderPass, std::vector<std::vector<vk::ImageView>> attachments, uint32_t width, uint32_t height, uint32_t layers) {
		std::vector<vk::Framebuffer> framebuffers(attachments.size());
		for (uint32_t i = 0; i < attachments.size(); i++) {
			framebuffers[i] = createFrameBuffer(renderPass, attachments[i], width, height, layers);
		}
		return FramebufferSet(m_device, framebuffers, { width, height });
	}
}