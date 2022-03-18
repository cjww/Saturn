#include "pch.h"
#include "Renderer.hpp"

namespace NAME_SPACE {

	void Renderer::setupDebug() {
		m_validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		uint32_t count;
		VkResult res = vkEnumerateInstanceLayerProperties(&count, nullptr);
		vbl::printError(res, "Failed to enumerate instance layers");
		std::vector<VkLayerProperties> layerProperties(count);
		res = vkEnumerateInstanceLayerProperties(&count, layerProperties.data());
		vbl::printError(res, "Failed to enumerate instance layers");


		for (int i = 0; i < m_validationLayers.size(); i++) {
			bool found = false;
			for (const auto& prop : layerProperties) {
				if (strcmp(m_validationLayers[i], prop.layerName) == 0) {
					found = true;
					break;
				}
			}
			if (!found) {
				std::cerr << "A validation layer requested was not present in available layers: " << m_validationLayers[i] << std::endl;
				m_validationLayers.erase(m_validationLayers.begin() + i);
				i--;
			}
		}

		vbl::setValidationLayers(true, m_validationLayers.data(), m_validationLayers.size());
	}

	void Renderer::createInstance() {

		m_appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		m_appInfo.pNext = nullptr;
		m_appInfo.apiVersion = VK_API_VERSION_1_2;
		//TODO: read from file
		m_appInfo.applicationVersion = 0;
		m_appInfo.engineVersion = 0;
		m_appInfo.pApplicationName = "My Application";
		m_appInfo.pEngineName = "No Engine";

		uint32_t count = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
		m_instanceExtensions.resize(count);
		for (uint32_t i = 0; i < count; i++) {
			m_instanceExtensions[i] = glfwExtensions[i];
		}
		m_instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		m_instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

		vbl::printError(
			vbl::createInstance(&m_instance, m_instanceExtensions.data(), m_instanceExtensions.size(), &m_appInfo),
			"Failed to create instance",
			true
		);
	}

	void Renderer::getPhysicalDevice() {
		vbl::printError(
			vbl::getPhysicalDevices(m_instance, &m_physicalDevice, 1),
			"No GPU found",
			true
		);
	}

	void Renderer::createDevice() {

		m_deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		m_graphicsQueueInfo = vbl::getQueueInfo(m_physicalDevice, VK_QUEUE_GRAPHICS_BIT, 1);
		m_computeQueueInfo = vbl::getQueueInfo(m_physicalDevice, VK_QUEUE_COMPUTE_BIT, 1);

		std::vector<vbl::QueueInfo> queueInfos;
		if (m_graphicsQueueInfo.family == m_computeQueueInfo.family) {
			m_graphicsQueueInfo = vbl::getQueueInfo(m_physicalDevice, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 2);
			queueInfos.push_back(m_graphicsQueueInfo);
		}
		else {
			queueInfos.push_back(m_graphicsQueueInfo);
			queueInfos.push_back(m_computeQueueInfo);
		}

		VkPhysicalDeviceFeatures features = {};
		vkGetPhysicalDeviceFeatures(m_physicalDevice, &features);

		vbl::printError(
			vbl::createLogicalDevice(&m_device, m_physicalDevice, m_deviceExtensions.data(), m_deviceExtensions.size(), queueInfos.data(), queueInfos.size(), &features),
			"Failed to create device", true);


		vkGetDeviceQueue(m_device, m_graphicsQueueInfo.family, 0, &m_graphicsQueue);
		if (m_graphicsQueueInfo.queueCount != 1) {
			vkGetDeviceQueue(m_device, m_computeQueueInfo.family, 1, &m_computeQueue);
		}
		else {
			m_computeQueue = m_graphicsQueue;
		}

	}


	void Renderer::createCommandPools() {
		vbl::printError(
			vbl::createCommandPool(&m_graphicsCommandPool, m_device, m_graphicsQueueInfo.family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT),
			"Failed to create command pool for graphics queue"
		);
		vbl::printError(
			vbl::createCommandPool(&m_computeCommandPool, m_device, m_computeQueueInfo.family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT),
			"Failed to create command pool for compute queue"
		);
	}

	void Renderer::createCommandBuffers() {

		m_computeCommandBuffers.resize(1);
		vbl::allocateCommandBuffers(m_computeCommandBuffers.data(), m_computeCommandBuffers.size(), m_device, m_computeCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		m_transferCommandBuffers.resize(m_inFlightCount * 8);
		vbl::allocateCommandBuffers(m_transferCommandBuffers.data(), m_transferCommandBuffers.size(), m_device, m_graphicsCommandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

	}

	void Renderer::createGraphicsCommandBuffers() {
		m_graphicsCommandBuffers.resize(m_inFlightCount);
		vbl::allocateCommandBuffers(m_graphicsCommandBuffers.data(), m_graphicsCommandBuffers.size(), m_device, m_graphicsCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	}

	VkFramebuffer Renderer::createFramebuffer(VkExtent2D extent, VkRenderPass renderPass, const std::vector<VkImageView>& imageViews) {
		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.width = extent.width;
		info.height = extent.height;
		info.layers = 1;
		info.pAttachments = imageViews.data();
		info.attachmentCount = imageViews.size();
		info.renderPass = renderPass;

		VkFramebuffer framebuffer;
		vbl::printError(
			vkCreateFramebuffer(m_device, &info, nullptr, &framebuffer),
			"Failed to create Framebuffer"
		);
		return framebuffer;
	}

	Renderer::Renderer()
	{
		
#ifdef _DEBUG
		setupDebug();
#endif
		createInstance();
		getPhysicalDevice();
		createDevice();

		m_pDataManager = new DataManager(m_instance, m_device, m_physicalDevice, m_appInfo.apiVersion, { m_graphicsQueueInfo.family }, { m_computeQueueInfo.family });
		
		m_inFlightCount = 3;

		createCommandPools();
		createCommandBuffers();
		createGraphicsCommandBuffers();

		createSyncronisationObjects();

	}

	void Renderer::destroySwapchains() {
		for (auto& swapchain : m_swapchains)
		{
			if (swapchain.swapchain == VK_NULL_HANDLE)
				continue;

			for (uint32_t i = 0; i < m_inFlightCount; i++) {
				vkDestroyImageView(m_device, swapchain.imageViews[i], nullptr);
			}
			vkDestroySwapchainKHR(m_device, swapchain.swapchain, nullptr);
			vkDestroySurfaceKHR(m_instance, swapchain.surface, nullptr);
		}
		m_swapchains.clear();
	}

	void Renderer::createImGUIDescriptorPool() {
		VkDescriptorPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;

		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER,					1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,	1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,				1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,				1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,		1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,		1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,			1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,			1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,	1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,	1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,			1000 }
		};

		info.pPoolSizes = poolSizes;
		info.poolSizeCount = 11;
		info.maxSets = 1000;
		vbl::printError(
			vkCreateDescriptorPool(m_device, &info, nullptr, &m_imGuiDescriptorPool),
			"Failed to create ImGui descriptorPool"
		);
	}

	ImGui_ImplVulkan_InitInfo Renderer::getImGUIInitInfo() const {
		ImGui_ImplVulkan_InitInfo i = {};
		memset(&i, 0, sizeof(i));
		i.Instance = m_instance;
		i.PhysicalDevice = m_physicalDevice;
		i.Device = m_device;
		i.QueueFamily = m_graphicsQueueInfo.family;
		i.Queue = m_graphicsQueue;
		i.PipelineCache = VK_NULL_HANDLE;
		i.DescriptorPool = m_imGuiDescriptorPool;
		i.Allocator = nullptr;
		i.MinImageCount = m_inFlightCount;
		i.ImageCount = m_inFlightCount;
		i.CheckVkResultFn = nullptr;
		return i;
	}

	void Renderer::destroySyncronisationObjects() {
		for (uint32_t i = 0; i < m_inFlightCount; i++) {
			vkDestroySemaphore(m_device, m_imageAvailableSemaphore[i], nullptr);
			vkDestroySemaphore(m_device, m_renderFinishedSemaphore[i], nullptr);
			vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
		}
		m_imageAvailableSemaphore.clear();
		m_renderFinishedSemaphore.clear();
		m_inFlightFences.clear();

		m_imageFences.clear();

	}

	
	void Renderer::freeGraphicsCommandBuffers() {
		vkFreeCommandBuffers(m_device, m_graphicsCommandPool, m_graphicsCommandBuffers.size(), m_graphicsCommandBuffers.data());
	}

	Renderer::~Renderer() {
		vkDeviceWaitIdle(m_device);

		
		destroyRenderPasses();
		destroyFramebuffers();
		destroyPipelines();
		destroySwapchains();

		destroySyncronisationObjects();
		
		vkDestroyCommandPool(m_device, m_graphicsCommandPool, nullptr);
		vkDestroyCommandPool(m_device, m_computeCommandPool, nullptr);

		delete m_pDataManager;

		vkDestroyDevice(m_device, nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}

	Renderer& Renderer::get() {
		static Renderer instance;
		return instance;
	}

	void Renderer::initImGUI(GLFWwindow* window, uint32_t renderpass, uint32_t subpass) {
		IMGUI_CHECKVERSION();
		auto context = ImGui::CreateContext();
		ImGui::SetCurrentContext(context);
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImGui::StyleColorsDark();

		createImGUIDescriptorPool();
		auto info = getImGUIInitInfo();
		info.Subpass = subpass;
		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_Init(&info, m_renderPasses[renderpass].renderPass);

		VkCommandBuffer commandBuffer;
		vbl::allocateCommandBuffers(&commandBuffer, 1, m_device, m_graphicsCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = nullptr;
		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		
		
		vkEndCommandBuffer(commandBuffer);
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_graphicsQueue);

		vkFreeCommandBuffers(m_device, m_graphicsCommandPool, 1, &commandBuffer);
	}

	void Renderer::newFrameImGUI() {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void Renderer::endFrameImGUI() {
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_graphicsCommandBuffers[m_frameIndex]);
	}

	void Renderer::cleanupImGUI() {
		vkDeviceWaitIdle(m_device);
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		vkDestroyDescriptorPool(m_device, m_imGuiDescriptorPool, nullptr);
	}

	ImTextureID Renderer::getImTextureID(Texture* texture, const SamplerPtr& sampler) {
		
		if (m_imGuiImages.find(texture) == m_imGuiImages.end()) {
			m_imGuiImages.insert(std::make_pair(texture, ImGui_ImplVulkan_AddTexture(*sampler, texture->view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)));
		}
		return m_imGuiImages.at(texture);
	}

	uint32_t Renderer::getNextSwapchainImage(uint32_t swapchainIndex) {
		/*
		if (m_window->wasResized()) {
			//recreate(getCurrentExtent());
			m_window->setWasResized(false);
		}
		if (m_window->isIconified()) {
			return -1;
		}
		*/

		Swapchain& swapchain = m_swapchains.at(swapchainIndex);

		vkWaitForFences(m_device, 1, &m_inFlightFences[m_frameIndex], VK_FALSE, UINT64_MAX);

		VkResult res = vkAcquireNextImageKHR(m_device, swapchain.swapchain, UINT64_MAX, m_imageAvailableSemaphore[m_frameIndex], VK_NULL_HANDLE, &swapchain.currentImageIndex);
		if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
			invokeSwapchainResize(swapchainIndex);
			return -1;
		}
		else {
			vbl::printError(
				res,
				"Failed to acquire image"
			);

			if (m_imageFences[swapchain.currentImageIndex] != VK_NULL_HANDLE) {
				vkWaitForFences(m_device, 1, &m_imageFences[swapchain.currentImageIndex], VK_FALSE, UINT64_MAX);
			}
			m_imageFences[swapchain.currentImageIndex] = m_inFlightFences[m_frameIndex];
		}

		return swapchain.currentImageIndex;
	}

	bool Renderer::beginFrame(uint32_t swapchain) {

		if (swapchain != -1) {
			if (getNextSwapchainImage(swapchain) == -1) {
				return false;
			}
		}

		vbl::beginPrimaryCommandBuffer(m_graphicsCommandBuffers[m_frameIndex], VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		if (!m_transferCommandQueue.empty()) {
			std::vector<VkCommandBuffer> transferBuffers;
			transferBuffers.reserve(m_transferCommandQueue.size());
			for (const auto& t : m_transferCommandQueue) {
				transferBuffers.push_back(t.commandBuffer);
			}
			vkCmdExecuteCommands(m_graphicsCommandBuffers[m_frameIndex], transferBuffers.size(), transferBuffers.data());
		}
		
		return true;
	}

	void Renderer::endFrame() {
		vkEndCommandBuffer(m_graphicsCommandBuffers[m_frameIndex]);
	}

	VkSurfaceKHR Renderer::createSurface(GLFWwindow* window) {
		VkSurfaceKHR surface;
		vbl::printError(
			glfwCreateWindowSurface(m_instance, window, nullptr, &surface),
			"Failed to create surface",
			true
		);
		return surface;
	}

	uint32_t Renderer::createSwapchain(GLFWwindow* window) {

		m_swapchains.push_back({});
		Swapchain& swapchain = m_swapchains.back();
		swapchain.surface = createSurface(window);
		vbl::printError(
			vbl::createSwapchain(&swapchain.swapchain, m_device, m_physicalDevice, swapchain.surface, &m_graphicsQueueInfo, 1, &swapchain.format),
			"Failed to create swap chain",
			true
		);

		VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
		vbl::printError(
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, swapchain.surface, &surfaceCapabilities)
		);
		swapchain.extent = surfaceCapabilities.currentExtent;

		uint32_t count;
		vkGetSwapchainImagesKHR(m_device, swapchain.swapchain, &count, nullptr);
		swapchain.images.resize(count);
		m_inFlightCount = count;
		vkGetSwapchainImagesKHR(m_device, swapchain.swapchain, &count, swapchain.images.data());

		swapchain.imageViews.resize(count);
		for (uint32_t i = 0; i < m_inFlightCount; i++) {
			m_pDataManager->createImageView(swapchain.imageViews[i],
				VK_IMAGE_VIEW_TYPE_2D,
				swapchain.images[i],
				swapchain.format,
				VK_IMAGE_ASPECT_COLOR_BIT,
				0,
				0
			);
		}

		return m_swapchains.size() - 1;
	}

	void Renderer::recreateSwapchain(uint32_t swapchain) {
		
		vkDeviceWaitIdle(m_device);

		Swapchain& s = m_swapchains.at(swapchain);

		vbl::printError(
			vbl::createSwapchain(&s.swapchain, m_device, m_physicalDevice, s.surface, &m_graphicsQueueInfo, 1, &s.format),
			"Failed to create swap chain",
			true
		);
		destroySyncronisationObjects();

		//vkDestroySwapchainKHR(m_device, s.swapchain, nullptr);
		for (auto& imageView : s.imageViews) {
			vkDestroyImageView(m_device, imageView, nullptr);
		}

		VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
		vbl::printError(
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, s.surface, &surfaceCapabilities)
		);
		s.extent = surfaceCapabilities.currentExtent;

		uint32_t count;
		vkGetSwapchainImagesKHR(m_device, s.swapchain, &count, nullptr);
		s.images.resize(count);
		m_inFlightCount = count;
		vkGetSwapchainImagesKHR(m_device, s.swapchain, &count, s.images.data());
		
		s.currentImageIndex = 0;

		s.imageViews.resize(count);
		for (uint32_t i = 0; i < m_inFlightCount; i++) {
			m_pDataManager->createImageView(s.imageViews[i],
				VK_IMAGE_VIEW_TYPE_2D,
				s.images[i],
				s.format,
				VK_IMAGE_ASPECT_COLOR_BIT,
				0,
				0
			);
		}
		createSyncronisationObjects();
	}

	void Renderer::setOnSwapchainResizeCallback(uint32_t swapchain, ResizeCallbackFunc function) {
		m_swapchainResizeCallbacks[swapchain] = function;
	}

	void Renderer::invokeSwapchainResize(uint32_t swapchain) {
		if (m_swapchainResizeCallbacks.count(swapchain)) {
			VkSurfaceCapabilitiesKHR capabilities;
			vbl::printError(
				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_swapchains.at(swapchain).surface, &capabilities),
				"Failed to get surface capabilities"
			);
			m_swapchainResizeCallbacks.at(swapchain)(capabilities.currentExtent.width, capabilities.currentExtent.height);
		}
	}

	void Renderer::submit() {
		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &m_graphicsCommandBuffers[m_frameIndex];
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = &m_renderFinishedSemaphore[m_frameIndex];
		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = &m_imageAvailableSemaphore[m_frameIndex];

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		info.pWaitDstStageMask = &waitStage;

		vkResetFences(m_device, 1, &m_inFlightFences[m_frameIndex]);

		vkQueueSubmit(m_graphicsQueue, 1, &info, m_inFlightFences[m_frameIndex]);
		m_transferCommandQueue.clear();
		m_frameIndex = (m_frameIndex + 1) % m_inFlightCount;

	}

	void Renderer::present(uint32_t swapchain) {
		Swapchain& s = m_swapchains.at(swapchain);
		uint32_t prevFrame = (m_frameIndex + (m_inFlightCount - 1)) % m_inFlightCount;
		vbl::presentImage(m_graphicsQueue, s.currentImageIndex, s.swapchain, m_renderFinishedSemaphore[prevFrame]);
	}

	void Renderer::createSyncronisationObjects() {
		m_frameIndex = 0;
		m_inFlightFences.resize(m_inFlightCount);
		vbl::printError(
			vbl::createFences(m_inFlightFences.data(), m_inFlightFences.size(), m_device, VK_FENCE_CREATE_SIGNALED_BIT),
			"Failed to create fences",
			true
		);

		m_imageFences.resize(m_inFlightFences.size(), VK_NULL_HANDLE);

		m_imageAvailableSemaphore.resize(m_inFlightCount);
		vbl::printError(
			vbl::createSemaphores(m_imageAvailableSemaphore.data(), m_imageAvailableSemaphore.size(), m_device),
			"Failed to create semaphores",
			true
		);

		m_renderFinishedSemaphore.resize(m_inFlightCount);
		vbl::printError(
			vbl::createSemaphores(m_renderFinishedSemaphore.data(), m_renderFinishedSemaphore.size(), m_device),
			"Failed to create semaphores",
			true
		);
	}

	

	uint32_t Renderer::createRenderPass(const std::vector<VkAttachmentDescription>& attachments,
		const std::vector<VkSubpassDescription>& subpasses,
		const std::vector<VkSubpassDependency>& dependencies)
	{

		RenderPass renderPass;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.flags = 0;
		renderPassInfo.pNext = nullptr;
		renderPassInfo.attachmentCount = attachments.size();
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = subpasses.size();
		renderPassInfo.pSubpasses = subpasses.data();
		renderPassInfo.dependencyCount = dependencies.size();
		renderPassInfo.pDependencies = dependencies.data();
		/*
		renderPass.colorAttachmentCount.resize(subpasses.size());
		renderPass.depthAttachment.resize(subpasses.size());
		for (uint32_t i = 0; i < (uint32_t)subpasses.size(); i++) {
			renderPass.colorAttachmentCount[i] = subpasses[i].colorAttachmentCount;
			renderPass.depthAttachment[i] = subpasses[i].pDepthStencilAttachment != nullptr;
		}
		*/

		for (uint32_t i = 0; i < (uint32_t)attachments.size(); i++) {
			if (attachments[i].format >= VK_FORMAT_UNDEFINED && attachments[i].format < VK_FORMAT_D16_UNORM) {
				// Color attachment
				VkClearValue clearValue;
				clearValue.color = { 0, 0, 0, 0 };
				renderPass.clearValues.push_back(clearValue);
			}
			else {
				// Depth attachment
				VkClearValue clearValue;
				clearValue.depthStencil.depth = 1.0f;
				clearValue.depthStencil.stencil = 0;
				renderPass.clearValues.push_back(clearValue);

			}
		}

		VkResult res = vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &renderPass.renderPass);
		if (res != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Renderpass");
		}

		renderPass.subpassCount = subpasses.size();

		m_renderPasses.emplace_back(renderPass);
		return m_renderPasses.size() - 1;
	}

	VkAttachmentDescription Renderer::getSwapchainAttachment(uint32_t swapchin) const {
		return getResolveAttachment(m_swapchains.at(swapchin).format);
	}

	uint32_t Renderer::createSwapchainFramebuffer(uint32_t swapchain, uint32_t renderPass, const std::vector<Texture*>& additionalAttachments) {
		Framebuffer framebuffer;
		framebuffer.framebuffers.resize(m_inFlightCount);
		for (uint32_t i = 0; i < m_inFlightCount; i++) {
			framebuffer.imageViews.resize(additionalAttachments.size() + 1);
			framebuffer.imageViews[0] = m_swapchains.at(swapchain).imageViews[i];
			for (uint32_t j = 0; j < (uint32_t)additionalAttachments.size(); j++) {
				framebuffer.imageViews[j + 1] = additionalAttachments[j]->view;
			}

			framebuffer.framebuffers[i] = createFramebuffer(m_swapchains.at(swapchain).extent, m_renderPasses.at(renderPass).renderPass, framebuffer.imageViews);
		}		
		framebuffer.extent = m_swapchains.at(swapchain).extent;

		m_framebuffers.push_back(framebuffer);
		return m_framebuffers.size() - 1;
	}


	uint32_t Renderer::createFramebuffer(uint32_t renderPass, VkExtent2D extent, const std::vector<Texture*>& attachments) {
		Framebuffer framebuffer;
		framebuffer.framebuffers.resize(m_inFlightCount);
		for (uint32_t i = 0; i < m_inFlightCount; i++) {
			
			framebuffer.imageViews.resize(attachments.size());
			for (uint32_t j = 0; j < (uint32_t)attachments.size(); j++) {
				framebuffer.imageViews[j] = attachments[j]->view;
			}

			framebuffer.framebuffers[i] = createFramebuffer(extent, m_renderPasses[renderPass].renderPass, framebuffer.imageViews);
		}
		framebuffer.extent = extent;

		m_framebuffers.push_back(framebuffer);
		return m_framebuffers.size() - 1;
	}


	uint32_t Renderer::createGraphicsPipeline(VkExtent2D extent, uint32_t renderPass, uint32_t subpassIndex,
		const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
		const std::vector<VkPushConstantRange>& pushConstantRanges,
		const std::vector<VkPipelineShaderStageCreateInfo>& shaderStages,
		VkPipelineVertexInputStateCreateInfo vertexInput,
		vbl::PipelineConfig config
		)
	{
		Pipeline pipeline;
		vbl::printError(
			vbl::createPipelineLayout(&pipeline.layout, m_device, 
				descriptorSetLayouts.data(), descriptorSetLayouts.size(), 
				pushConstantRanges.data(), pushConstantRanges.size()),
			"Failed to create pipeline layout"
		);

		vbl::printError(
			vbl::createGraphicsPipeline(
				&pipeline.pipeline,
				m_device,
				pipeline.layout,
				m_renderPasses[renderPass].renderPass,
				subpassIndex,
				extent,
				shaderStages.data(),
				shaderStages.size(),
				vertexInput,
				config
			),
			"Failed to create pipeline"
		);
		
		pipeline.isCompute = false;

		m_pipelines.push_back(pipeline);
		return m_pipelines.size() - 1;
	}

	uint32_t Renderer::createComputePipeline(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
		const std::vector<VkPushConstantRange>& pushConstantRanges, VkPipelineShaderStageCreateInfo shaderStage) 
	{

		Pipeline pipeline;
		vbl::printError(
			vbl::createPipelineLayout(&pipeline.layout, m_device, descriptorSetLayouts.data(), descriptorSetLayouts.size(), pushConstantRanges.data(), pushConstantRanges.size()),
			"Failed to create pipeline layout"
		);

		vbl::printError(
			vbl::createComputePipeline(&pipeline.pipeline, m_device, pipeline.layout, shaderStage),
			"Faield to create compute pipeline"
		);
	
		pipeline.isCompute = true;

		m_pipelines.push_back(pipeline);
		return m_pipelines.size() - 1;
	}

	VkCommandBuffer Renderer::beginTransferCommand(const Framebuffer& framebuffer, const RenderPass& renderPass, uint32_t subpass) {

		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.pNext = nullptr;
		inheritanceInfo.occlusionQueryEnable = false;
		inheritanceInfo.pipelineStatistics = 0;
		inheritanceInfo.queryFlags = 0;

		inheritanceInfo.framebuffer = framebuffer.framebuffers[(m_frameIndex + 1) % m_inFlightCount];
		inheritanceInfo.renderPass = renderPass.renderPass;
		inheritanceInfo.subpass = subpass;

		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.pNext = nullptr;
		info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		info.pInheritanceInfo = &inheritanceInfo;

		if (m_transferCommandQueue.size() == m_transferCommandBuffers.size()) {
			//throw std::runtime_error("Transfer overflow"); //TODO Fix this
			size_t oldSize = m_transferCommandBuffers.size();
			m_transferCommandBuffers.resize(m_transferCommandBuffers.size() << 1);
			uint32_t diff = m_transferCommandBuffers.size() - oldSize;
			vbl::printError(
				vbl::allocateCommandBuffers(&m_transferCommandBuffers[oldSize], diff, m_device, m_graphicsCommandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY),
				"Failed to allocate Commandbuffers"
			);

		}

		VkCommandBuffer commandBuffer = m_transferCommandBuffers[m_transferCommandQueue.size()];
		vkBeginCommandBuffer(commandBuffer, &info);
		return commandBuffer;
	}

	void Renderer::endTransferCommand(const TransferCommand& command) {
		vkEndCommandBuffer(command.commandBuffer);
		m_transferCommandQueue.push_back(command);
	}

	void Renderer::destroyFramebuffers() {
		for (auto& framebuffer : m_framebuffers) {
			for (uint32_t i = 0; i < m_inFlightCount; i++) {
				vkDestroyFramebuffer(m_device, framebuffer.framebuffers[i], nullptr);
			}
		}
	}

	void Renderer::destroyRenderPasses() {
		for (auto& renderpass : m_renderPasses) {
			vkDestroyRenderPass(m_device, renderpass.renderPass, nullptr);
		}
	}

	void Renderer::destroyPipelines() {
		for (auto& pipeline : m_pipelines) {
			vkDestroyPipelineLayout(m_device, pipeline.layout, nullptr);
			vkDestroyPipeline(m_device, pipeline.pipeline, nullptr);
		}
	}

	Texture* Renderer::createDepthTexture(VkExtent2D extent) {
		return m_pDataManager->createDepthAttachmentTexture2D(
			extent,
			VK_SAMPLE_COUNT_1_BIT);
	}

	Texture* Renderer::createTexture2D(uint32_t framebuffer, uint32_t renderpass, uint32_t subpass, VkExtent2D extent, unsigned char* pixels, int channels) {
		auto image = m_pDataManager->createColorTexture2D(
			extent,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			1,
			1);

		Buffer* buffer = m_pDataManager->createBuffer(image->extent.width * image->extent.height * channels, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, pixels);

		queueTransferCommand(
			framebuffer,
			renderpass,
			subpass,
			buffer,
			image);


		return image;
	}

	Texture* Renderer::createColorAttachmentTexture(VkExtent2D extent, VkFormat format, uint32_t arrayLayers, uint32_t mipLevels, VkSampleCountFlagBits sampleCount, VkImageUsageFlags additionalUsage) {
		auto image = m_pDataManager->createImage(
			{ extent.width, extent.height, 1 },
			arrayLayers,
			format,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_LAYOUT_UNDEFINED,
			mipLevels,
			{ m_graphicsQueueInfo.family, m_computeQueueInfo.family },
			sampleCount,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | additionalUsage,
			VMA_MEMORY_USAGE_GPU_ONLY,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		m_pDataManager->createImageView(
			VK_IMAGE_VIEW_TYPE_2D,
			image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			0);

		return image;
	}

	Texture* Renderer::createTexture3D(VkExtent3D extent, VkFormat format)
	{
		auto image = m_pDataManager->createTexture3D(
			extent,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			format,
			1,
			1);

		return image;

	}

	void Renderer::updateTexture(Texture* dst, uint32_t framebuffer, uint32_t renderpass, uint32_t subpass, void* data, size_t size) {
		Buffer* buffer = m_pDataManager->createBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, data);
		queueTransferCommand(
			framebuffer,
			renderpass,
			subpass,
			buffer,
			dst);
	}


	void Renderer::queueTransferCommand(uint32_t framebuffer, uint32_t renderpass, uint32_t subpass, Buffer* srcBuffer, Texture* dstTexture) {

		VkCommandBuffer commandBuffer = beginTransferCommand(m_framebuffers.at(framebuffer), m_renderPasses.at(renderpass), subpass);

		VkImageMemoryBarrier copy_barrier = {};
		copy_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		copy_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		copy_barrier.oldLayout = dstTexture->layout;
		copy_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		copy_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		copy_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		copy_barrier.image = dstTexture->image;
		copy_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy_barrier.subresourceRange.levelCount = 1;
		copy_barrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &copy_barrier);

		VkBufferImageCopy copy = {};
		copy.imageExtent = dstTexture->extent;
		copy.imageOffset = { 0, 0, 0 };
		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.imageSubresource.baseArrayLayer = 0;
		copy.imageSubresource.layerCount = 1;
		copy.imageSubresource.mipLevel = 0;

		copy.bufferImageHeight = 0;
		copy.bufferOffset = 0;
		copy.bufferRowLength = 0;
		
		vkCmdCopyBufferToImage(commandBuffer, srcBuffer->buffer, dstTexture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

		VkImageMemoryBarrier use_barrier = {};
		use_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		use_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		use_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		use_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		use_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		use_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		use_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		use_barrier.image = dstTexture->image;
		use_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		use_barrier.subresourceRange.levelCount = 1;
		use_barrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &use_barrier);


		TransferCommand transferCommand = {};
		transferCommand.commandBuffer = commandBuffer;
		transferCommand.srcBuffer = srcBuffer;
		transferCommand.dstImage = dstTexture;
		transferCommand.type = TransferCommand::Type::BUFFER_TO_IMAGE;
		endTransferCommand(transferCommand);

		dstTexture->layout = use_barrier.newLayout;
	}

	/*
	void Renderer::queueTransferCommand(Buffer* srcBuffer, Buffer* dstBuffer) {

		VkCommandBuffer commandBuffer = beginTransferCommand();

		VkBufferCopy bufferCopy = {};
		bufferCopy.dstOffset = 0;
		bufferCopy.size = srcBuffer->size;
		bufferCopy.srcOffset = 0;

		vkCmdCopyBuffer(commandBuffer, srcBuffer->buffer, dstBuffer->buffer, 1, &bufferCopy);
		
		TransferCommand transferCommand = {};
		transferCommand.commandBuffer = commandBuffer;
		transferCommand.srcBuffer = srcBuffer;
		transferCommand.dstBuffer = dstBuffer;
		transferCommand.type = TransferCommand::Type::BUFFER_TO_BUFFER;
		endTransferCommand(transferCommand);
	}

	void Renderer::queueTransferCommand(Texture* srcTexture, Buffer* dstBuffer)
	{

		VkCommandBuffer commandBuffer = beginTransferCommand();

		VkBufferImageCopy copy = {};
		copy.imageExtent = srcTexture->extent;
		copy.imageOffset = { 0, 0, 0 };
		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.imageSubresource.baseArrayLayer = 0;
		copy.imageSubresource.layerCount = 1;
		copy.imageSubresource.mipLevel = 0;

		copy.bufferImageHeight = 0;
		copy.bufferOffset = 0;
		copy.bufferRowLength = 0;
		vkCmdCopyImageToBuffer(commandBuffer, srcTexture->image, srcTexture->layout, dstBuffer->buffer, 1, &copy);

		TransferCommand transferCommand = {};
		transferCommand.commandBuffer = commandBuffer;
		transferCommand.dstBuffer = dstBuffer;
		transferCommand.srcImage = srcTexture;
		transferCommand.type = TransferCommand::Type::IMAGE_TO_BUFFER;
		endTransferCommand(transferCommand);
	}

	void Renderer::queueTransferCommand(Texture* srcTexture, Texture* dstTexture) {

		VkCommandBuffer commandBuffer = beginTransferCommand();

		VkImageCopy imageCopy = {};
		imageCopy.dstOffset = {0, 0, 0};
		imageCopy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopy.dstSubresource.baseArrayLayer = 0;
		imageCopy.dstSubresource.layerCount = 1;
		imageCopy.dstSubresource.mipLevel = 0;
		imageCopy.extent = srcTexture->extent;
		
		imageCopy.srcOffset = {0, 0, 0};
		imageCopy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageCopy.srcSubresource.baseArrayLayer = 0;
		imageCopy.srcSubresource.layerCount = 1;
		imageCopy.srcSubresource.mipLevel = 0;
	
		vkCmdCopyImage(commandBuffer, srcTexture->image, srcTexture->layout, dstTexture->image, dstTexture->layout, 1, &imageCopy);
	
		TransferCommand transferCommand = {};
		transferCommand.commandBuffer = commandBuffer;
		transferCommand.srcImage = srcTexture;
		transferCommand.dstImage = dstTexture;
		transferCommand.type = TransferCommand::Type::IMAGE_TO_IMAGE;
		endTransferCommand(transferCommand);
	}
	*/

	SamplerPtr Renderer::createSampler(VkFilter minMagFilter, float maxAnisotropy, float minLod, float maxLod, float mipLodBias) {
		SamplerPtr sampler(new VkSampler, [&](VkSampler* sampler) {
			vkDeviceWaitIdle(m_device);
			vkDestroySampler(m_device, *sampler, nullptr);
			delete sampler;
			});

		vbl::printError(
			vbl::createSampler(sampler.get(), m_device, minMagFilter, maxAnisotropy, minLod, maxLod, mipLodBias),
			"Failed to create sampler"
		);
		return sampler;
	}

	SamplerPtr Renderer::createSampler(VkSamplerCreateInfo info) {
		SamplerPtr sampler(new VkSampler, [&](VkSampler* sampler) {
			vkDeviceWaitIdle(m_device);
			vkDestroySampler(m_device, *sampler, nullptr);
			delete sampler;
		});

		vbl::printError(
			vkCreateSampler(m_device, &info, nullptr, sampler.get()),
			"Failed to create sampler"
		);
		return sampler;
	}

	Buffer* Renderer::createVertexBuffer(VkDeviceSize size, void* initialData) {
		return m_pDataManager->createBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, initialData);
	}

	Buffer* Renderer::createIndexBuffer(VkDeviceSize size, void* initialData) {
		return m_pDataManager->createBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, initialData);
	}

	Buffer* Renderer::createUniformBuffer(VkDeviceSize size, void* initialData) {
		return m_pDataManager->createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, initialData);
	}

	Buffer* Renderer::createStorageBuffer(VkDeviceSize size, void* initialData) {
		return m_pDataManager->createBuffer(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, initialData);
	}

	void Renderer::destroyBuffer(Buffer* buffer) {
		vkDeviceWaitIdle(m_device);
		m_pDataManager->destroyBuffer(buffer);
	}

	void Renderer::destroyTexture(Texture* texture) {
		vkDeviceWaitIdle(m_device);
		m_pDataManager->destroyImage(texture);
	}

	ShaderPtr Renderer::createShader(const char* path, VkShaderStageFlagBits stage) {
		return std::make_shared<Shader>(m_device, path, stage);
	}

	uint32_t Renderer::createPipeline(VkExtent2D extent, const ShaderSetPtr& shaderSet, uint32_t renderPass, uint32_t subpassIndex, vbl::PipelineConfig config) {
		if (shaderSet->isGraphicsSet()) {

			auto vertexAttributes = shaderSet->getVertexAttributes();
			auto vertexBindings = shaderSet->getVertexBindings();

			VkPipelineVertexInputStateCreateInfo inputState = {};
			inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			inputState.pNext = nullptr;
			inputState.flags = 0;
			inputState.vertexAttributeDescriptionCount = vertexAttributes.size();
			inputState.pVertexAttributeDescriptions = vertexAttributes.data();
			inputState.vertexBindingDescriptionCount = vertexBindings.size();
			inputState.pVertexBindingDescriptions = vertexBindings.data();

			return createGraphicsPipeline(
				extent,
				renderPass,
				subpassIndex,
				shaderSet->getDescriptorSetLayouts(),
				shaderSet->getPushConstantRanges(),
				shaderSet->getShaderInfos(),
				inputState,
				config
			);
		}
		else {
			//Compute pipeline
			return createComputePipeline(shaderSet->getDescriptorSetLayouts(), shaderSet->getPushConstantRanges(), shaderSet->getShaderInfos()[0]);
		}

	}

	uint32_t Renderer::createPipeline(uint32_t swapchain, const ShaderSetPtr& shaderSet, uint32_t renderPass, uint32_t subpassIndex, vbl::PipelineConfig config) {
		if (swapchain == -1)
			return createPipeline({ 0, 0 }, shaderSet, renderPass, subpassIndex, config);
		return createPipeline(m_swapchains.at(swapchain).extent, shaderSet, renderPass, subpassIndex, config);
	}

	ShaderSetPtr Renderer::createShaderSet(const ShaderPtr& vertexShader, const ShaderPtr& fragmentShader) {
		return std::shared_ptr<ShaderSet>(new ShaderSet(m_device, m_inFlightCount, vertexShader, fragmentShader), [&](ShaderSet* shaderSet) {
			vkQueueWaitIdle(m_graphicsQueue);
			delete shaderSet;
		});
	}

	ShaderSetPtr Renderer::createShaderSet(const ShaderPtr& vertexShader, const ShaderPtr& geometryShader, const ShaderPtr& fragmentShader) {
		return std::shared_ptr<ShaderSet>(new ShaderSet(m_device, m_inFlightCount, vertexShader, geometryShader, fragmentShader), [&](ShaderSet* shaderSet) {
			vkQueueWaitIdle(m_graphicsQueue);
			delete shaderSet;
		});
	}

	ShaderSetPtr Renderer::createShaderSet(const ShaderPtr& computeShader) {
		return std::shared_ptr<ShaderSet>(new ShaderSet(m_device, m_inFlightCount, computeShader), [&](ShaderSet* shaderSet) {
			vkQueueWaitIdle(m_computeQueue);
			delete shaderSet;
		});
	}


	CommandBufferPtr Renderer::createCommandBuffer(bool isCompute) {
		CommandBufferPtr commandBuffer = std::make_shared<CommandBuffer>();

		commandBuffer->buffers.resize(m_inFlightCount);
		vbl::allocateCommandBuffers(
			commandBuffer->buffers.data(),
			commandBuffer->buffers.size(),
			m_device, 
			(isCompute) ? m_computeCommandPool : m_graphicsCommandPool,
			(isCompute) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY);

		return commandBuffer;
	}

	void Renderer::recordCommandBuffer(const CommandBufferPtr& commandBuffer, uint32_t framebuffer, uint32_t renderPass, uint32_t subpassIndex, std::function<void(uint32_t frameIndex)> drawCalls, bool isOneTimeRecord) {
		if (isOneTimeRecord) {
			vbl::beginSecondaryCommandBuffer(
				commandBuffer->buffers[m_frameIndex],
				VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
				m_framebuffers[framebuffer].framebuffers[m_frameIndex],
				m_renderPasses[renderPass].renderPass,
				subpassIndex
			);

			drawCalls(m_frameIndex);
			vkEndCommandBuffer(commandBuffer->buffers[m_frameIndex]);
		}
		else {
			for (uint32_t i = 0; i < (uint32_t)commandBuffer->buffers.size(); i++) {
				vbl::beginSecondaryCommandBuffer(
					commandBuffer->buffers[i],
					VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
					m_framebuffers[framebuffer].framebuffers[i],
					m_renderPasses[renderPass].renderPass,
					subpassIndex
				);
				drawCalls(i);
				vkEndCommandBuffer(commandBuffer->buffers[i]);
			}
		}
	}

	void Renderer::recordCommandBuffer(const CommandBufferPtr& commandBuffer, std::function<void(uint32_t frameIndex)> drawCalls, bool isOneTimeRecord) {
		if (isOneTimeRecord) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.pNext = nullptr;
			beginInfo.pInheritanceInfo = nullptr;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
			vkBeginCommandBuffer(commandBuffer->buffers[m_frameIndex], &beginInfo);

			drawCalls(m_frameIndex);
			vkEndCommandBuffer(commandBuffer->buffers[m_frameIndex]);
		}
		else {
			for (uint32_t i = 0; i < (uint32_t)commandBuffer->buffers.size(); i++) {
				VkCommandBufferBeginInfo beginInfo = {};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.pNext = nullptr;
				beginInfo.pInheritanceInfo = nullptr;
				beginInfo.flags = 0;
				vkBeginCommandBuffer(commandBuffer->buffers[i], &beginInfo);

				drawCalls(i);
				vkEndCommandBuffer(commandBuffer->buffers[i]);
			}
		}

	}

	void Renderer::executeCommandBuffer(const CommandBufferPtr& commandBuffer, bool isCompute) {
		vkCmdExecuteCommands((isCompute) ? m_computeCommandBuffers[0] : m_graphicsCommandBuffers[m_frameIndex], 1, &commandBuffer->buffers[m_frameIndex]);
	}

	void Renderer::executeCommandBuffers(const std::vector<CommandBufferPtr>& commandBuffers, bool isCompute) {
		std::vector<VkCommandBuffer> cmdBuffers(commandBuffers.size());
		for (uint32_t i = 0; i < (uint32_t)cmdBuffers.size(); i++) {
			cmdBuffers[i] = commandBuffers[i]->buffers[m_frameIndex];
		}
		vkCmdExecuteCommands((isCompute) ? m_computeCommandBuffers[0] : m_graphicsCommandBuffers[m_frameIndex], cmdBuffers.size(), cmdBuffers.data());
	}

	std::shared_ptr<VkFence> Renderer::submitToComputeQueue(const CommandBufferPtr& commandBuffer) {
		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.pNext = nullptr;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &commandBuffer->buffers[m_frameIndex];
		info.signalSemaphoreCount = 0;
		info.pSignalSemaphores = nullptr;
		info.pWaitDstStageMask = nullptr;
		info.waitSemaphoreCount = 0;
		info.pWaitSemaphores = nullptr;
		std::shared_ptr<VkFence> fence(new VkFence, [&](VkFence* fence) {
			vkDestroyFence(m_device, *fence, nullptr);
			delete fence;
		});

		vbl::printError(
			vbl::createFence(fence.get(), m_device),
			"Failed to create Fence"
		);
		vbl::printError(
			vkQueueSubmit(m_computeQueue, 1, &info, *fence),
			"Failed to submit to queue"
		);

		return fence;
	}

	void Renderer::waitForFence(std::shared_ptr<VkFence> fence, uint64_t timeout) {
		vkWaitForFences(m_device, 1, fence.get(), VK_TRUE, timeout);
	}

	void Renderer::updateDescriptorSet(const DescriptorSetPtr& descriptorSet, uint32_t binding, const Buffer* buffer, const Texture* image, const SamplerPtr& sampler, bool isOneTimeUpdate) {

		if (descriptorSet->writes.size() <= binding) {
			DEBUG_LOG_ERROR("Binding", binding, "out of bounds!");
			throw std::runtime_error("Binding out of bounds!");
		}

		VkDescriptorBufferInfo bufferInfo = {};
		VkDescriptorImageInfo imageInfo = {};

		if (buffer != nullptr) {
			bufferInfo.buffer = buffer->buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = buffer->size;
			descriptorSet->writes[binding].pBufferInfo = &bufferInfo;
			descriptorSet->writes[binding].pImageInfo = nullptr;
			descriptorSet->writes[binding].pTexelBufferView = nullptr;

		}
		else if (image != nullptr) {
			imageInfo.imageLayout = image->layout;
			imageInfo.imageView = image->view;
			imageInfo.sampler = sampler == nullptr ? VK_NULL_HANDLE : *sampler;
			descriptorSet->writes[binding].pImageInfo = &imageInfo;
			descriptorSet->writes[binding].pBufferInfo = nullptr;
			descriptorSet->writes[binding].pTexelBufferView = nullptr;
		}

		if (isOneTimeUpdate) {
			std::vector<VkWriteDescriptorSet> writes;
			for (uint32_t i = 0; i < descriptorSet->descriptorSets.size(); i++) {
				descriptorSet->writes[binding].dstSet = descriptorSet->descriptorSets[i];
				descriptorSet->writes[binding].dstArrayElement = 0;
				writes.push_back(descriptorSet->writes[binding]);
			}
			vkUpdateDescriptorSets(m_device, writes.size(), writes.data(), 0, nullptr);
		}
		else {
			descriptorSet->writes[binding].dstSet = descriptorSet->descriptorSets[m_frameIndex];
			descriptorSet->writes[binding].dstArrayElement = 0;

			vkUpdateDescriptorSets(m_device, 1, &descriptorSet->writes[binding], 0, nullptr);
		}
		
	}

	void Renderer::beginRenderPass(uint32_t renderPass, uint32_t framebuffer, VkSubpassContents contents, glm::vec3 clearColor) {
		VkRenderPassBeginInfo beginInfo = {};
		beginInfo.pNext = nullptr;
		beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

		beginInfo.clearValueCount = m_renderPasses[renderPass].clearValues.size();
		beginInfo.pClearValues = m_renderPasses[renderPass].clearValues.data();
		beginInfo.framebuffer = m_framebuffers[framebuffer].framebuffers[m_frameIndex];
		
		VkRect2D renderArea = {};
		renderArea.extent = m_framebuffers.at(framebuffer).extent;
		renderArea.offset = { 0, 0 };

		beginInfo.renderArea = renderArea;
		beginInfo.renderPass = m_renderPasses[renderPass].renderPass;

		vkCmdBeginRenderPass(m_graphicsCommandBuffers[m_frameIndex], &beginInfo, contents);
	}

	void Renderer::nextSubpass(VkSubpassContents contents) {
		vkCmdNextSubpass(m_graphicsCommandBuffers[m_frameIndex], contents);
	}

	void Renderer::endRenderPass() {
		vkCmdEndRenderPass(m_graphicsCommandBuffers[m_frameIndex]);
	}
	
	void Renderer::transferTextureLayout(Texture* texture, VkImageLayout newLayout, const CommandBufferPtr& commandBuffer, uint32_t frameIndex) {
		uint32_t realFrameIndex = (frameIndex == -1) ? m_frameIndex : frameIndex;

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.oldLayout = texture->layout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = texture->image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(m_graphicsCommandBuffers[realFrameIndex], VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
	}

	void Renderer::bindPipeline(uint32_t pipeline, const CommandBufferPtr& commandBuffer, uint32_t frameIndex) {
		auto p = m_pipelines[pipeline];
		uint32_t realFrameIndex = (frameIndex == -1) ? m_frameIndex : frameIndex;
		if (p.isCompute) {
			vkCmdBindPipeline((commandBuffer == nullptr)? m_computeCommandBuffers[0] : commandBuffer->buffers[realFrameIndex], VK_PIPELINE_BIND_POINT_COMPUTE, p.pipeline);
		}
		else {
			vkCmdBindPipeline((commandBuffer == nullptr) ? m_graphicsCommandBuffers[realFrameIndex] : commandBuffer->buffers[realFrameIndex] , VK_PIPELINE_BIND_POINT_GRAPHICS, p.pipeline);
		}
	}

	void Renderer::bindVertexBuffer(const Buffer* vertexBuffer, const CommandBufferPtr& commandBuffer, uint32_t frameIndex) {
		VkDeviceSize offset = 0;
		uint32_t realFrameIndex = (frameIndex == -1) ? m_frameIndex : frameIndex;
		vkCmdBindVertexBuffers((commandBuffer == nullptr)? m_graphicsCommandBuffers[realFrameIndex] : commandBuffer->buffers[realFrameIndex], 0, 1, &vertexBuffer->buffer, &offset);
	}

	void Renderer::bindVertexBuffers(const std::vector<Buffer*>& vertexBuffers, const CommandBufferPtr& commandBuffer, uint32_t frameIndex) {

		uint32_t realFrameIndex = (frameIndex == -1) ? m_frameIndex : frameIndex;
		std::vector<VkDeviceSize> offsets(vertexBuffers.size());
		std::vector<VkBuffer> buffers(vertexBuffers.size());
		int i = 0;
		for (auto& buffer : buffers) {
			buffer = vertexBuffers[i]->buffer;
			offsets[i] = 0;
			i++;
		}

		vkCmdBindVertexBuffers((commandBuffer == nullptr) ? m_graphicsCommandBuffers[realFrameIndex] : commandBuffer->buffers[realFrameIndex],
			0, buffers.size(), buffers.data(), offsets.data());
	}

	void Renderer::bindIndexBuffer(const Buffer* indexbuffer, const CommandBufferPtr& commandBuffer, uint32_t frameIndex) {
		uint32_t realFrameIndex = (frameIndex == -1) ? m_frameIndex : frameIndex;
		vkCmdBindIndexBuffer((commandBuffer == nullptr) ? m_graphicsCommandBuffers[realFrameIndex] : commandBuffer->buffers[realFrameIndex], indexbuffer->buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void Renderer::bindViewports(const std::vector<VkViewport>& viewports, const CommandBufferPtr& commandBuffer, uint32_t frameIndex) {
		uint32_t realFrameIndex = (frameIndex == -1) ? m_frameIndex : frameIndex;
		vkCmdSetViewport((commandBuffer == nullptr) ? m_graphicsCommandBuffers[realFrameIndex] : commandBuffer->buffers[realFrameIndex], 0, viewports.size(), viewports.data());
	}

	void Renderer::waitDeviceIdle() const {
		vkDeviceWaitIdle(m_device);
	}

	void Renderer::bindViewport(const VkViewport& viewport, const CommandBufferPtr& commandBuffer, uint32_t frameIndex) {
		uint32_t realFrameIndex = (frameIndex == -1) ? m_frameIndex : frameIndex;
		vkCmdSetViewport((commandBuffer == nullptr) ? m_graphicsCommandBuffers[realFrameIndex] : commandBuffer->buffers[realFrameIndex], 0, 1, &viewport);
	}

	void Renderer::bindDescriptorSet(const DescriptorSetPtr& descriptorSet, uint32_t pipeline, const CommandBufferPtr& commandBuffer, uint32_t frameIndex) {
		uint32_t realFrameIndex = (frameIndex == -1) ? m_frameIndex : frameIndex;
		if (m_pipelines[pipeline].isCompute) {
			vkCmdBindDescriptorSets((commandBuffer == nullptr) ? m_computeCommandBuffers[realFrameIndex] : commandBuffer->buffers[realFrameIndex],
				VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelines[pipeline].layout, descriptorSet->setIndex, 1, &descriptorSet->descriptorSets[realFrameIndex], 0, nullptr);
		}
		else {
			vkCmdBindDescriptorSets((commandBuffer == nullptr) ? m_graphicsCommandBuffers[realFrameIndex] : commandBuffer->buffers[realFrameIndex],
				VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines[pipeline].layout, descriptorSet->setIndex, 1, &descriptorSet->descriptorSets[realFrameIndex], 0, nullptr);
		}
	}

	void Renderer::pushConstants(uint32_t pipeline, VkShaderStageFlags shaderStages, uint32_t offset, uint32_t size, void* data, const CommandBufferPtr& commandBuffer, uint32_t frameIndex) {
		uint32_t realFrameIndex = (frameIndex == -1) ? m_frameIndex : frameIndex;
		vkCmdPushConstants((commandBuffer == nullptr) ? m_graphicsCommandBuffers[realFrameIndex] : commandBuffer->buffers[realFrameIndex],
			m_pipelines[pipeline].layout, shaderStages, offset, size, data);
	}

	void Renderer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance, const CommandBufferPtr& commandBuffer, uint32_t frameIndex) {
		uint32_t realFrameIndex = (frameIndex == -1) ? m_frameIndex : frameIndex;
		vkCmdDraw((commandBuffer == nullptr)? m_graphicsCommandBuffers[realFrameIndex] : commandBuffer->buffers[realFrameIndex], vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void Renderer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t firstInstance, uint32_t vertexOffset, const CommandBufferPtr& commandBuffer, uint32_t frameIndex) {
		uint32_t realFrameIndex = (frameIndex == -1) ? m_frameIndex : frameIndex;
		vkCmdDrawIndexed((commandBuffer == nullptr) ? m_graphicsCommandBuffers[realFrameIndex] : commandBuffer->buffers[realFrameIndex], indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

}