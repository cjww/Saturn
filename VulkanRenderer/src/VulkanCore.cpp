#include "pch.h"
#include "VulkanCore.hpp"

#include "debugFunctions.hpp" // for checkError and debugCallback

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
		m_formats.push_back({ FormatPrecisionFlagBits::e8Bit, FormatDimensionFlagBits::e3, FormatTypeFlagBits::UNORM, vk::Format::eB8G8R8Unorm });
	}


	uint32_t VulkanCore::getQueueFamilyIndex(vk::QueueFlags capabilities, vk::QueueFamilyProperties* prop) {
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();

		for (uint32_t i = 0; i < (uint32_t)queueFamilyProperties.size(); i++) {
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
			"VK_LAYER_KHRONOS_validation",
			"VK_LAYER_KHRONOS_synchronization2",
			"VK_LAYER_LUNARG_monitor",
			"VK_LAYER_GOOGLE_threading"
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
				SA_DEBUG_LOG_WARNING("A validation layer requested was not present in available layers: ", m_validationLayers[i]);
				m_validationLayers.erase(m_validationLayers.begin() + i);
				i--;
			}
		}
	}

	void VulkanCore::createInstance() {
		uint32_t count = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
		if (count == 0) {
			glfwInit();
			glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
		}
		for (uint32_t i = 0; i < count; i++) {
			m_instanceExtensions.push_back(glfwExtensions[i]);
		}
		m_instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		m_instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

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
		m_deviceExtensions.push_back(VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);
		m_deviceExtensions.push_back(VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME);

		m_queueInfo = getQueueInfo(vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute, FRAMES_IN_FLIGHT);
		std::vector<QueueInfo> queueInfos = { m_queueInfo };
	
		vk::PhysicalDeviceFeatures features = m_physicalDevice.getFeatures();
		vk::PhysicalDeviceDescriptorIndexingFeatures indexingFeatures;

		indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		indexingFeatures.runtimeDescriptorArray = VK_TRUE;
		indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
		indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
		

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
		for (const auto& queueInfo : queueInfos) {
			queueCreateInfos.push_back(vk::DeviceQueueCreateInfo {
				.queueFamilyIndex = queueInfo.family,
				.queueCount = queueInfo.queueCount,
				.pQueuePriorities = queueInfo.priorities.data()
			});
		}

		vk::DeviceCreateInfo deviceInfo{
			.pNext = &indexingFeatures,
			.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
			.pQueueCreateInfos = queueCreateInfos.data(),
			.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size()),
			.ppEnabledExtensionNames = m_deviceExtensions.data(),
			.pEnabledFeatures = &features,
		};
		
		if (!m_validationLayers.empty()) {
			deviceInfo.setPEnabledLayerNames(m_validationLayers);
		}


		m_device = m_physicalDevice.createDevice(deviceInfo);
		m_queues.resize(m_queueInfo.queueCount);
		for (uint32_t i = 0; i < m_queueInfo.queueCount; i++) {
			m_queues[i] = m_device.getQueue(m_queueInfo.family, 0);
		}
	}

	void VulkanCore::createCommandPool() {
		m_mainCommandPool.create(m_device, m_queueInfo.family, vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
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
		
		createCommandPool();

		m_memoryManager.create(m_instance, m_device, m_physicalDevice, m_appInfo.apiVersion);

		fillFormats();

		m_defaultColorFormat = vk::Format::eR8G8B8A8Srgb;
		m_defaultDepthFormat = getSupportedDepthFormat();
	}

	void VulkanCore::cleanup() {
		cleanupImGui();

		m_memoryManager.destroy();
		m_mainCommandPool.destroy();

		m_device.destroy();
		m_instance.destroy();
	}
#ifndef IMGUI_DISABLE
	void VulkanCore::initImGui(GLFWwindow* pWindow, vk::RenderPass renderPass, uint32_t subpass) {

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		
		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui_ImplGlfw_InitForVulkan(pWindow, true);
		
		vk::DescriptorPoolSize poolSizes[] =
		{
			{ vk::DescriptorType::eSampler, 1000 },
			{ vk::DescriptorType::eCombinedImageSampler, 1000 },
			{ vk::DescriptorType::eSampledImage, 1000 },
			{ vk::DescriptorType::eStorageImage, 1000 },
			{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
			{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
			{ vk::DescriptorType::eUniformBuffer, 1000 },
			{ vk::DescriptorType::eStorageBuffer, 1000 },
			{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
			{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
			{ vk::DescriptorType::eInputAttachment, 1000 }
		};
		vk::DescriptorPoolCreateInfo poolInfo = {
			.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			.maxSets = 1000 * IM_ARRAYSIZE(poolSizes),
			.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes),
			.pPoolSizes = poolSizes,
		};
		m_imGuiDescriptorPool = m_device.createDescriptorPool(poolInfo);
		

		ImGui_ImplVulkan_InitInfo info = {
			.Instance = m_instance,
			.PhysicalDevice = m_physicalDevice,
			.Device = m_device,
			.QueueFamily = m_queueInfo.family,
			.Queue = m_queues[0],
			.PipelineCache = VK_NULL_HANDLE,
			.DescriptorPool = m_imGuiDescriptorPool,
			.Subpass = subpass,
			.MinImageCount = 2,
			.ImageCount = (uint32_t)m_queues.size(),
			.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
			.Allocator = nullptr,
			.CheckVkResultFn = [](VkResult result) { checkError((vk::Result)result, "Failed to initialize ImGui with vulkan", false); },
		};
		
		ImGui_ImplVulkan_Init(&info, renderPass);
		CommandBufferSet commandBuffer = m_mainCommandPool.allocateOneTimeCommandBuffer(m_queues[0], vk::CommandBufferLevel::ePrimary);
		commandBuffer.begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer.getBuffer());

		commandBuffer.end();
		commandBuffer.submit();
		m_queues[0].waitIdle();
		commandBuffer.destroy();	
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void VulkanCore::cleanupImGui() {
		if (m_imGuiDescriptorPool) {
			m_device.destroyDescriptorPool(m_imGuiDescriptorPool);
			m_imGuiDescriptorPool = VK_NULL_HANDLE;
		}
		m_imGuiImages.clear();
	}

	VkDescriptorSet VulkanCore::getImGuiImageDescriptoSet(vk::ImageView* imageView, vk::ImageLayout layout) {

		if (layout != vk::ImageLayout::eShaderReadOnlyOptimal &&
			layout != vk::ImageLayout::eReadOnlyOptimal &&
			layout != vk::ImageLayout::eAttachmentOptimal &&
			layout != vk::ImageLayout::eGeneral &&
			layout != vk::ImageLayout::eDepthStencilAttachmentOptimal &&
			layout != vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal &&
			layout != vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal &&
			layout != vk::ImageLayout::eDepthReadOnlyOptimal &&
			layout != vk::ImageLayout::eStencilReadOnlyOptimal) 
		{
			layout = vk::ImageLayout::eShaderReadOnlyOptimal;
		}
		VkDescriptorSet texture = VK_NULL_HANDLE;
		if (m_imGuiImages.count(imageView)) {
			texture = m_imGuiImages.at(imageView);
		}
		else {
			texture = ImGui_ImplVulkan_AddTexture(m_imGuiImageSampler, *imageView, (VkImageLayout)layout);
			m_imGuiImages[imageView] = texture;
		}
		return texture;
		
	}

#endif

	vk::SurfaceKHR VulkanCore::createSurface(GLFWwindow* pWindow) {
		VkSurfaceKHR surface;
		checkError(
			(vk::Result)glfwCreateWindowSurface(m_instance, pWindow, nullptr, &surface),
			"Failed to create window surface",
			true
		);
		return surface;
	}

	vk::SwapchainKHR VulkanCore::createSwapchain(vk::SurfaceKHR surface, vk::Format* outFormat, vk::Extent2D* outExtent) {
		if (!m_physicalDevice.getSurfaceSupportKHR(m_queueInfo.family, surface)) {
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
			.pQueueFamilyIndices = &m_queueInfo.family,
			.preTransform = surfaceCapabilities.currentTransform,
			.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode = presentMode,
			.clipped = true,
		};

		if(outFormat) *outFormat = finalFormat.format;
		if (outExtent) *outExtent = surfaceCapabilities.currentExtent;

		return m_device.createSwapchainKHR(info);
	}

	vk::ImageView VulkanCore::createImageView(vk::ImageViewType type, vk::Image image, vk::Format format, vk::ImageAspectFlags aspectMask, uint32_t mipLevels, uint32_t baseMipLevel, uint32_t layers, uint32_t baseArrayLevel) {
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
				.levelCount = mipLevels,
				.baseArrayLayer = baseArrayLevel,
				.layerCount = layers
			}
		};

		return m_device.createImageView(info);
	}

	vk::BufferView VulkanCore::createBufferView(vk::Buffer buffer, vk::Format format) {
		vk::BufferViewCreateInfo info = {
			.flags = (vk::BufferViewCreateFlags)0,
			.buffer = buffer,
			.format = format,
			.offset = 0,
			.range = VK_WHOLE_SIZE,
		};
		return m_device.createBufferView(info);
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
			.sampleShadingEnable = config.multisample.sampleShadingEnable,
			.minSampleShading = config.multisample.minSampleShading,
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

	CommandBufferSet VulkanCore::allocateCommandBufferSet(vk::CommandBufferLevel level) {
		return m_mainCommandPool.allocateCommandBufferSet(m_queues, level);
	}

	CommandBufferSet VulkanCore::allocateCommandBufferSet(vk::CommandBufferLevel level, CommandPool& commandPool) {
		return commandPool.allocateCommandBufferSet(m_queues, level);
	}

	DeviceBuffer* VulkanCore::createBuffer(vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags allocationFlags, size_t size, void* initialData) {
		return m_memoryManager.createBuffer(size, usage, memoryUsage, allocationFlags, initialData);
	}

	void VulkanCore::destroyBuffer(DeviceBuffer* pBuffer) {
		m_memoryManager.destroyBuffer(pBuffer);
	}

	DeviceImage* VulkanCore::createImage2D(Extent extent, vk::Format format, vk::ImageUsageFlags usage, vk::SampleCountFlagBits sampleCount, uint32_t mipLevels, uint32_t arrayLayers, vk::ImageCreateFlags flags) {
		return m_memoryManager.createImage(
			{ extent.width, extent.height, 1 },
			arrayLayers,
			format,
			vk::ImageType::e2D,
			vk::ImageLayout::eUndefined,
			mipLevels,
			{ m_queueInfo.family },
			sampleCount,
			vk::SharingMode::eExclusive,
			vk::ImageTiling::eOptimal,
			usage,
			VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			flags
		);
	}

	DeviceImage* VulkanCore::createImage3D(Extent3D extent, vk::Format format, vk::ImageUsageFlags usage, vk::SampleCountFlagBits sampleCount, uint32_t mipLevels, uint32_t arrayLayers, vk::ImageCreateFlags flags) {
		return m_memoryManager.createImage(
			{ extent.width, extent.height, extent.depth },
			arrayLayers,
			format,
			vk::ImageType::e3D,
			vk::ImageLayout::eUndefined,
			mipLevels,
			{ m_queueInfo.family },
			sampleCount,
			vk::SharingMode::eExclusive,
			vk::ImageTiling::eOptimal,
			usage,
			VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
			vk::MemoryPropertyFlagBits::eDeviceLocal,
			flags
		);
	}

	void VulkanCore::destroyImage(DeviceImage* pImage) {
		m_memoryManager.destroyImage(pImage);
	}

	void VulkanCore::transferImageLayout(vk::CommandBuffer commandBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, vk::Image image, vk::ImageAspectFlags imageAspect, uint32_t mipLevels, uint32_t layers, vk::PipelineStageFlags srcStage, vk::PipelineStageFlags dstStage) {
		vk::ImageMemoryBarrier imageBarrier{
			.srcAccessMask = srcAccessMask,
			.dstAccessMask = dstAccessMask,
			.oldLayout = oldLayout,
			.newLayout = newLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange{
				.aspectMask = imageAspect,
				.levelCount = mipLevels,
				.layerCount = layers,
			},
		};

		commandBuffer.pipelineBarrier(
			srcStage,
			dstStage,
			(vk::DependencyFlags)0,
			nullptr,
			nullptr,
			imageBarrier);
	}


	void VulkanCore::transferBufferToColorImage(vk::CommandBuffer commandBuffer, vk::Buffer buffer, vk::Image image, uint32_t mipLevels, uint32_t layers, vk::Extent3D copyExtent, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::AccessFlags dstAccessMask, vk::PipelineStageFlags dstStage) {
		transferImageLayout(commandBuffer,
			oldLayout,
			vk::ImageLayout::eTransferDstOptimal,
			(vk::AccessFlags)0,
			vk::AccessFlagBits::eTransferWrite,
			image, 
			vk::ImageAspectFlagBits::eColor,
			mipLevels,
			layers,
			vk::PipelineStageFlagBits::eHost,
			vk::PipelineStageFlagBits::eTransfer);

		std::vector<vk::BufferImageCopy> regions;

		for (uint32_t i = 0; i < layers; i++) {
			vk::DeviceSize offset = copyExtent.width * copyExtent.height * copyExtent.depth * 4 * i; // TODO: warning, assumption
			vk::BufferImageCopy copy{
				.bufferOffset = offset,
				.imageSubresource{
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.mipLevel = 0,
					.baseArrayLayer = i,
					.layerCount = 1,
				},
				.imageOffset = { 0, 0, 0 },
				.imageExtent = copyExtent,
			};
			regions.push_back(copy);
		}

		commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, regions);

		if (mipLevels > 1) {
			generateMipmaps(commandBuffer, image, copyExtent, mipLevels);
			return;
		}
		transferImageLayout(commandBuffer,
			vk::ImageLayout::eTransferDstOptimal,
			newLayout,
			vk::AccessFlagBits::eTransferWrite,
			dstAccessMask,
			image, 
			vk::ImageAspectFlagBits::eColor,
			mipLevels,
			layers,
			vk::PipelineStageFlagBits::eTransfer,
			dstStage);
	
	}

	void VulkanCore::generateMipmaps(vk::CommandBuffer commandBuffer, vk::Image image, vk::Extent3D extent, uint32_t mipLevels) {
		vk::ImageMemoryBarrier barrier = {
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = {
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};

		vk::Extent3D mipExtent = extent;
		for (uint32_t i = 1; i < mipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
			barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

			commandBuffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eTransfer,
				(vk::DependencyFlags)0, 
				nullptr, 
				nullptr, 
				barrier);

			vk::ImageBlit blit = {
				.srcSubresource = {
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.mipLevel = i - 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
				.dstSubresource = {
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.mipLevel = i,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
			};
			blit.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
			blit.srcOffsets[1] = vk::Offset3D{
				(int32_t)mipExtent.width,
				(int32_t)mipExtent.height,
				(int32_t)mipExtent.depth
			};

			if (mipExtent.width > 1) mipExtent.width /= 2;
			if (mipExtent.height > 1) mipExtent.height /= 2;
			if (mipExtent.depth > 1) mipExtent.depth /= 2;


			blit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
			blit.dstOffsets[1] = vk::Offset3D{
				(int32_t)mipExtent.width,
				(int32_t)mipExtent.height,
				(int32_t)mipExtent.depth
			};

			commandBuffer.blitImage(
				image, vk::ImageLayout::eTransferSrcOptimal, 
				image, vk::ImageLayout::eTransferDstOptimal, 
				blit, 
				vk::Filter::eLinear);

			barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
			barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			commandBuffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eFragmentShader,
				(vk::DependencyFlags)0,
				nullptr,
				nullptr,
				barrier);
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		commandBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eFragmentShader,
			(vk::DependencyFlags)0,
			nullptr,
			nullptr,
			barrier);

	}

	vk::Sampler VulkanCore::createSampler(const vk::SamplerCreateInfo& info) {
		return m_device.createSampler(info);
	}

	uint32_t VulkanCore::getQueueFamily() const {
		return m_queueInfo.family;
	}

	uint32_t VulkanCore::getQueueCount() const {
		return (uint32_t)m_queues.size();
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
			vk::Format::eD24UnormS8Uint,
			vk::Format::eD16UnormS8Uint,
			vk::Format::eD16Unorm,
			vk::Format::eD32SfloatS8Uint,
			vk::Format::eD32Sfloat,
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

	vk::SampleCountFlags VulkanCore::getSupportedColorSampleCounts() const {
		vk::PhysicalDeviceProperties properties = m_physicalDevice.getProperties();
		return properties.limits.framebufferColorSampleCounts;
	}

	vk::SampleCountFlags VulkanCore::getSupportedDepthSampleCounts() const {
		vk::PhysicalDeviceProperties properties = m_physicalDevice.getProperties();
		return properties.limits.framebufferDepthSampleCounts;
	}

	DeviceMemoryStats VulkanCore::getGPUMemoryUsage() const {
		return std::move(m_memoryManager.getDeviceMemoryStats());
	}

	void VulkanCore::setMemoryManagerFrameIndex(uint32_t frameIndex) {
		m_memoryManager.setCurrentFrameIndex(frameIndex);
	}

}