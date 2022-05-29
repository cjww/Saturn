#include "pch.h"
#include "VulkanCore.hpp"

#include "debugFunctions.h" // for checkError and debugCallback

namespace sa {
	void VulkanCore::fillFormats() {

		m_formats.push_back({ FormatPrecisionFlagBits::e32Bit, FormatDimensionFlagBits::e4, FormatTypeFlagBits::SFLOAT, vk::Format::eR32G32B32A32Sfloat });
		m_formats.push_back({ FormatPrecisionFlagBits::e32Bit, FormatDimensionFlagBits::e4, FormatTypeFlagBits::SINT, vk::Format::eR32G32B32A32Sint });
		m_formats.push_back({ FormatPrecisionFlagBits::e32Bit, FormatDimensionFlagBits::e4, FormatTypeFlagBits::UINT, vk::Format::eR32G32B32A32Uint });
		m_formats.push_back({ FormatPrecisionFlagBits::e16Bit, FormatDimensionFlagBits::e4, FormatTypeFlagBits::SFLOAT, vk::Format::eR16G16B16A16Sfloat });
		m_formats.push_back({ FormatPrecisionFlagBits::e16Bit, FormatDimensionFlagBits::e4, FormatTypeFlagBits::SINT, vk::Format::eR16G16B16A16Sint });
		m_formats.push_back({ FormatPrecisionFlagBits::e16Bit, FormatDimensionFlagBits::e4, FormatTypeFlagBits::UINT, vk::Format::eR16G16B16A16Uint });
		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e4, FormatTypeFlagBits::UNORM, vk::Format::eR8G8B8A8Unorm });
		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e4, FormatTypeFlagBits::SINT, vk::Format::eR8G8B8A8Sint });
		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e4, FormatTypeFlagBits::UINT, vk::Format::eR8G8B8A8Uint });
		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e4, FormatTypeFlagBits::SRGB, vk::Format::eR8G8B8A8Srgb });
		m_formats.push_back({ FormatPrecisionFlagBits::e32Bit, FormatDimensionFlagBits::e3, FormatTypeFlagBits::SFLOAT, vk::Format::eR32G32B32Sfloat });
		m_formats.push_back({ FormatPrecisionFlagBits::e32Bit, FormatDimensionFlagBits::e3, FormatTypeFlagBits::SINT, vk::Format::eR32G32B32Sint });
		m_formats.push_back({ FormatPrecisionFlagBits::e32Bit, FormatDimensionFlagBits::e3, FormatTypeFlagBits::UINT, vk::Format::eR32G32B32Uint });
		m_formats.push_back({ FormatPrecisionFlagBits::e16Bit, FormatDimensionFlagBits::e3, FormatTypeFlagBits::SFLOAT, vk::Format::eR16G16B16Sfloat });
		m_formats.push_back({ FormatPrecisionFlagBits::e16Bit, FormatDimensionFlagBits::e3, FormatTypeFlagBits::SINT, vk::Format::eR16G16B16Sint });
		m_formats.push_back({ FormatPrecisionFlagBits::e16Bit, FormatDimensionFlagBits::e3, FormatTypeFlagBits::UINT, vk::Format::eR16G16B16Uint });
		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e3, FormatTypeFlagBits::UNORM, vk::Format::eR8G8B8Unorm });
		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e3, FormatTypeFlagBits::SINT, vk::Format::eR8G8B8Sint });
		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e3, FormatTypeFlagBits::UINT, vk::Format::eR8G8B8Uint });
		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e3, FormatTypeFlagBits::SRGB, vk::Format::eR8G8B8Srgb });
		m_formats.push_back({ FormatPrecisionFlagBits::e32Bit, FormatDimensionFlagBits::e2, FormatTypeFlagBits::SFLOAT, vk::Format::eR32G32Sfloat });
		m_formats.push_back({ FormatPrecisionFlagBits::e32Bit, FormatDimensionFlagBits::e2, FormatTypeFlagBits::SINT, vk::Format::eR32G32Sint });
		m_formats.push_back({ FormatPrecisionFlagBits::e32Bit, FormatDimensionFlagBits::e2, FormatTypeFlagBits::UINT, vk::Format::eR32G32Uint });
		m_formats.push_back({ FormatPrecisionFlagBits::e16Bit, FormatDimensionFlagBits::e2, FormatTypeFlagBits::SFLOAT, vk::Format::eR16G16Sfloat });
		m_formats.push_back({ FormatPrecisionFlagBits::e16Bit, FormatDimensionFlagBits::e2, FormatTypeFlagBits::SINT, vk::Format::eR16G16Sint });
		m_formats.push_back({ FormatPrecisionFlagBits::e16Bit, FormatDimensionFlagBits::e2, FormatTypeFlagBits::UINT, vk::Format::eR16G16Uint });
		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e2, FormatTypeFlagBits::UNORM, vk::Format::eR8G8Unorm });
		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e2, FormatTypeFlagBits::SINT, vk::Format::eR8G8Sint });
		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e2, FormatTypeFlagBits::UINT, vk::Format::eR8G8Uint });
		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e2, FormatTypeFlagBits::SRGB, vk::Format::eR8G8Srgb });
		m_formats.push_back({ FormatPrecisionFlagBits::e32Bit, FormatDimensionFlagBits::e1, FormatTypeFlagBits::SFLOAT, vk::Format::eR32Sfloat });
		m_formats.push_back({ FormatPrecisionFlagBits::e32Bit, FormatDimensionFlagBits::e1, FormatTypeFlagBits::SINT, vk::Format::eR32Sint });
		m_formats.push_back({ FormatPrecisionFlagBits::e32Bit, FormatDimensionFlagBits::e1, FormatTypeFlagBits::UINT, vk::Format::eR32Uint });
		m_formats.push_back({ FormatPrecisionFlagBits::e16Bit, FormatDimensionFlagBits::e1, FormatTypeFlagBits::SFLOAT, vk::Format::eR16Sfloat });
		m_formats.push_back({ FormatPrecisionFlagBits::e16Bit, FormatDimensionFlagBits::e1, FormatTypeFlagBits::SINT, vk::Format::eR16Sint });
		m_formats.push_back({ FormatPrecisionFlagBits::e16Bit, FormatDimensionFlagBits::e1, FormatTypeFlagBits::UINT, vk::Format::eR16Uint });
		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e1, FormatTypeFlagBits::UNORM, vk::Format::eR8Unorm });
		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e1, FormatTypeFlagBits::SINT, vk::Format::eR8Sint });
		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e1, FormatTypeFlagBits::UINT, vk::Format::eR8Uint });
		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e1, FormatTypeFlagBits::SRGB, vk::Format::eR8Srgb });

		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e4, FormatTypeFlagBits::SRGB, vk::Format::eB8G8R8A8Srgb });
		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e4, FormatTypeFlagBits::UNORM, vk::Format::eB8G8R8A8Unorm });
		
	}


	uint32_t VulkanCore::getQueueFamilyIndex(vk::QueueFlags capabilities, vk::QueueFamilyProperties* prop) {
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();

		for (uint32_t i = (uint32_t)queueFamilyProperties.size() - 1; i >= 0; i--) {
			vk::QueueFamilyProperties& properties = queueFamilyProperties[i];
			if (properties.queueCount > 0) {
				if (properties.queueFlags & capabilities) {
					if (prop != nullptr) {
						*prop = properties;
					}
					return i;
				}
			}
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
		uint32_t count = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
		for (uint32_t i = 0; i < count; i++) {
			m_instanceExtensions.push_back(glfwExtensions[i]);
		}
		m_instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		vk::InstanceCreateInfo instanceInfo{
			.pApplicationInfo = &m_appInfo,
			.enabledExtensionCount = static_cast<uint32_t>(m_instanceExtensions.size()),
			.ppEnabledExtensionNames = m_instanceExtensions.data()
		};

		std::cout << "---Extensions---" << std::endl;
		for (const char* extension : m_instanceExtensions) {
			std::cout << extension << std::endl;
		}

		if (!m_validationLayers.empty()) {
			instanceInfo.setPEnabledLayerNames(m_validationLayers);
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
			deviceInfo.setPEnabledLayerNames(m_validationLayers);
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
		m_graphicsCommandPool.create(m_device, m_graphicsQueueInfo.family, m_graphicsQueue, vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
		m_computeCommandPool.create(m_device, m_computeQueueInfo.family, m_computeQueue, vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
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

	void VulkanCore::init(vk::ApplicationInfo appInfo) {
		m_appInfo = appInfo;
#ifdef _DEBUG
			setupDebug();
#endif // _DEBUG
		createInstance();
		findPhysicalDevice();
		createDevice();
		
		createCommandPools();

		m_memoryManager.create(m_instance, m_device, m_physicalDevice, m_appInfo.apiVersion, { m_graphicsQueueInfo.family }, { m_computeQueueInfo.family });

		fillFormats();

		m_defaultColorFormat = vk::Format::eR8G8B8A8Srgb;
		m_defaultDepthFormat = getSupportedDepthFormat();
	}

	void VulkanCore::cleanup() {
		
		m_memoryManager.destroy();

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

	FramebufferSet VulkanCore::createFrameBufferSet(vk::RenderPass renderPass, std::vector<std::vector<vk::ImageView>> attachments, uint32_t width, uint32_t height, uint32_t layers) {
		std::vector<vk::Framebuffer> framebuffers(attachments.size());
		for (uint32_t i = 0; i < attachments.size(); i++) {
			framebuffers[i] = createFrameBuffer(renderPass, attachments[i], width, height, layers);
		}
		return FramebufferSet(m_device, framebuffers, { width, height });
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

	CommandBufferSet VulkanCore::allocateGraphicsCommandBufferSet(uint32_t count, vk::CommandBufferLevel level) {
		return m_graphicsCommandPool.allocateCommandBufferSet(count, level);
	}

	CommandBufferSet VulkanCore::allocateComputeCommandBufferSet(uint32_t count, vk::CommandBufferLevel level) {
		return m_computeCommandPool.allocateCommandBufferSet(count, level);
	}

	DeviceBuffer* VulkanCore::createBuffer(vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocationFlags, size_t size, void* initialData) {
		return m_memoryManager.createBuffer(size, usage, memoryUsage, allocationFlags, initialData);
	}

	void VulkanCore::destroyBuffer(DeviceBuffer* pBuffer) {
		m_memoryManager.destroyBuffer(pBuffer);
	}

	DeviceImage* VulkanCore::createImage2D(Extent extent, vk::Format format, vk::ImageUsageFlags usage, vk::SampleCountFlagBits sampleCount, uint32_t mipLevels, uint32_t arrayLayers) {
		return m_memoryManager.createImage(
			{ extent.width, extent.height, 1 },
			arrayLayers,
			format,
			vk::ImageType::e2D,
			vk::ImageLayout::eUndefined,
			mipLevels,
			{ m_graphicsQueueInfo.family },
			sampleCount,
			vk::SharingMode::eExclusive,
			vk::ImageTiling::eOptimal,
			usage,
			VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
			vk::MemoryPropertyFlagBits::eDeviceLocal
		);
	}

	void VulkanCore::destroyImage(DeviceImage* pImage) {
		m_memoryManager.destroyImage(pImage);
	}

	void VulkanCore::transferImageLayout(vk::CommandBuffer commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, vk::Image image, vk::ImageAspectFlags imageAspect, vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage) {
		vk::ImageMemoryBarrier copyBarrier{
			.dstAccessMask = dstAccessMask,
			.oldLayout = oldLayout,
			.newLayout = newLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange{
				.aspectMask = imageAspect,
				.levelCount = 1,
				.layerCount = 1,
			},
		};

		commandBuffer.pipelineBarrier(
			srcStage,
			dstStage,
			(vk::DependencyFlags)0,
			nullptr,
			nullptr,
			copyBarrier);
	}


	void VulkanCore::transferBufferToColorImage(vk::CommandBuffer commandBuffer, vk::Buffer buffer, vk::Image image, vk::Extent3D copyExtent, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::AccessFlags dstAccessMask, vk::PipelineStageFlags dstStage) {
		transferImageLayout(commandBuffer,
			oldLayout,
			vk::ImageLayout::eTransferDstOptimal,
			(vk::AccessFlags)0,
			vk::AccessFlagBits::eTransferWrite,
			image, 
			vk::ImageAspectFlagBits::eColor,
			vk::PipelineStageFlagBits::eHost,
			vk::PipelineStageFlagBits::eTransfer);

		vk::BufferImageCopy copy{
			.imageSubresource{
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.imageOffset = { 0, 0, 0 },
			.imageExtent = copyExtent,
		};

		commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, copy);

		transferImageLayout(commandBuffer,
			vk::ImageLayout::eTransferDstOptimal,
			newLayout,
			vk::AccessFlagBits::eTransferWrite,
			dstAccessMask,
			image, 
			vk::ImageAspectFlagBits::eColor,
			vk::PipelineStageFlagBits::eTransfer,
			dstStage);
	}


	vk::Sampler VulkanCore::createSampler() {
		vk::SamplerCreateInfo info {

		};

		return m_device.createSampler(info);
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


	vk::Format VulkanCore::getSupportedDepthFormat() {
		std::vector<vk::Format> formats = {
			vk::Format::eD16UnormS8Uint,
			vk::Format::eD32SfloatS8Uint,
			vk::Format::eD24UnormS8Uint,
			vk::Format::eD32Sfloat,
			vk::Format::eD16Unorm,
		};
		vk::Format f = getFormat(
			formats,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment,
			vk::ImageTiling::eOptimal
		);
		if (f == vk::Format::eUndefined) {
			throw std::runtime_error("Could not find supported depth format");
		}
		return f;
	}

	vk::Format VulkanCore::getFormat(const std::vector<vk::Format>& candidates, vk::FormatFeatureFlags features, vk::ImageTiling tilling) {
		for (vk::Format format : candidates) {
			vk::FormatProperties properties = m_physicalDevice.getFormatProperties(format);

			switch (tilling)
			{
			case vk::ImageTiling::eOptimal:
				if (((properties.optimalTilingFeatures | properties.bufferFeatures) & features) == features) {
					return format;
				}
				break;
			case vk::ImageTiling::eLinear:
				if (((properties.linearTilingFeatures | properties.bufferFeatures) & features) == features) {
					return format;
				}
				break;
			default:
				break;
			}

		}
		return vk::Format::eUndefined;
	}

	vk::Format VulkanCore::getFormat(FormatPrecisionFlags precision, FormatDimensionFlags dimensions, FormatTypeFlags type, vk::FormatFeatureFlags features, vk::ImageTiling tilling) {
		std::vector<vk::Format> candidates;
		
		for (auto format : m_formats) {
			if (!(format.precision & precision)) {
				continue;
			}
			if (!(format.dimension & dimensions)) {
				continue;
			}
			if (!(format.type & type)) {
				continue;
			}
			candidates.push_back(format.format);
		}

		return getFormat(candidates, features, tilling);
	}

}