#include "Renderer.hpp"

namespace NAME_SPACE {

	void Renderer::setupDebug() {
		m_validationLayers = {
			"VK_LAYER_LUNARG_standard_validation",
			"VK_LAYER_KHRONOS_validation"
		};

		uint32_t count;
		VkResult res = vkEnumerateInstanceLayerProperties(&count, nullptr);
		vbl::printError(res, "Faiedl to enumerate instance layers");
		std::vector<VkLayerProperties> layerProperties(count);
		res = vkEnumerateInstanceLayerProperties(&count, layerProperties.data());
		vbl::printError(res, "Faiedl to enumerate instance layers");


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

	void Renderer::createSurface(GLFWwindow* window) {
		vbl::printError(
			glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface),
			"Failed to create surface"
		);
	}

	void Renderer::createSwapChain() {
		vbl::printError(
			vbl::createSwapchain(&m_swapChain.swapChain, m_device, m_physicalDevice, m_surface, &m_graphicsQueueInfo, 1, &m_swapChain.format),
			"Failed to create swap chain"
		);

		VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
		vbl::printError(
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceCapabilities)
		);
		m_swapChain.extent = surfaceCapabilities.currentExtent;


		uint32_t count;
		vkGetSwapchainImagesKHR(m_device, m_swapChain.swapChain, &count, nullptr);
		m_swapChain.images.resize(count);
		vkGetSwapchainImagesKHR(m_device, m_swapChain.swapChain, &count, m_swapChain.images.data());

		m_swapChain.imageViews.resize(count);
		for (uint32_t i = 0; i < (uint32_t)m_swapChain.images.size(); i++) {
			m_pResourceManager->createImageView(m_swapChain.imageViews[i],
				VK_IMAGE_VIEW_TYPE_2D,
				m_swapChain.images[i],
				m_swapChain.format,
				VK_IMAGE_ASPECT_COLOR_BIT,
				0,
				0
			);
		}
	}

	uint32_t Renderer::getNextSwapchainImage() {
		vkWaitForFences(m_device, 1, &m_inFlightFences[m_frameIndex], VK_FALSE, UINT64_MAX);
		
		VkResult res = vkAcquireNextImageKHR(m_device, m_swapChain.swapChain, UINT64_MAX, m_imageAvailableSemaphore[m_frameIndex], VK_NULL_HANDLE, &m_swapChain.currentImageIndex);
		if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
			return -1;
		}
		else {
			vbl::printError(
				res,
				"Failed to acquire image"
			);

			if (m_imageFences[m_swapChain.currentImageIndex] != VK_NULL_HANDLE) {
				vkWaitForFences(m_device, 1, &m_imageFences[m_swapChain.currentImageIndex], VK_FALSE, UINT64_MAX);
			}
			m_imageFences[m_swapChain.currentImageIndex] = m_inFlightFences[m_frameIndex];
		}

		return m_swapChain.currentImageIndex;
	}

	void Renderer::createCommandBuffers() {
		
		vbl::printError(
			vbl::createCommandPool(&m_graphicsCommandPool, m_device, m_graphicsQueueInfo.family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT),
			"Failed to create command pool for graphics queue"
		);
		vbl::printError(
			vbl::createCommandPool(&m_computeCommandPool, m_device, m_computeQueueInfo.family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT),
			"Failed to create command pool for compute queue"
		);

		m_graphicsCommandBuffers.resize(m_swapChain.images.size());
		vbl::allocateCommandBuffers(m_graphicsCommandBuffers.data(), m_graphicsCommandBuffers.size(), m_device, m_graphicsCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		
		m_computeCommandBuffers.resize(1);
		vbl::allocateCommandBuffers(m_computeCommandBuffers.data(), m_computeCommandBuffers.size(), m_device, m_computeCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

		m_transferCommandBuffers.resize(m_swapChain.images.size() * 8);
		vbl::allocateCommandBuffers(m_transferCommandBuffers.data(), m_transferCommandBuffers.size(), m_device, m_graphicsCommandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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

	Renderer::Renderer() {
		glfwInit();
		createInstance();
#ifdef _DEBUG
		setupDebug();
#endif
		getPhysicalDevice();
		createDevice();

		m_pResourceManager = new ResourceManager(m_instance, m_device, m_physicalDevice, m_appInfo.apiVersion, { m_graphicsQueueInfo.family }, { m_computeQueueInfo.family });
		
	}

	Renderer::~Renderer() {
		vkDeviceWaitIdle(m_device);

		delete m_pResourceManager;
		
		for (auto& renderPass : m_renderPasses) {
			vkDestroyRenderPass(m_device, renderPass.renderPass, nullptr);
		}

		for (auto& framebuffer : m_framebuffers) {
			for (uint32_t i = 0; i < (uint32_t)m_swapChain.images.size(); i++) {
				vkDestroyFramebuffer(m_device, framebuffer.framebuffers[i], nullptr);
			}
		}

		for (auto& pipeline : m_pipelines) {
			vkDestroyPipelineLayout(m_device, pipeline.layout, nullptr);
			vkDestroyPipeline(m_device, pipeline.pipeline, nullptr);
		}

		for (uint32_t i = 0; i < (uint32_t)m_swapChain.images.size(); i++) {
			vkDestroySemaphore(m_device, m_imageAvailableSemaphore[i], nullptr);
			vkDestroySemaphore(m_device, m_renderFinishedSemaphore[i], nullptr);
			vkDestroyFence(m_device, m_inFlightFences[i], nullptr);

			vkDestroyImageView(m_device, m_swapChain.imageViews[i], nullptr);
		}


		vkDestroyCommandPool(m_device, m_graphicsCommandPool, nullptr);
		vkDestroyCommandPool(m_device, m_computeCommandPool, nullptr);

		vkDestroySwapchainKHR(m_device, m_swapChain.swapChain, nullptr);
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		vkDestroyDevice(m_device, nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}

	void Renderer::createSyncronisationObjects() {
		m_frameIndex = 0;
		m_inFlightFences.resize(m_swapChain.images.size());
		vbl::printError(
			vbl::createFences(m_inFlightFences.data(), m_inFlightFences.size(), m_device, VK_FENCE_CREATE_SIGNALED_BIT),
			"Failed to create fences"
		);

		m_imageFences.resize(m_inFlightFences.size(), VK_NULL_HANDLE);

		m_imageAvailableSemaphore.resize(m_swapChain.images.size());
		vbl::printError(
			vbl::createSemaphores(m_imageAvailableSemaphore.data(), m_imageAvailableSemaphore.size(), m_device),
			"Failed to create semaphores"
		);

		m_renderFinishedSemaphore.resize(m_swapChain.images.size());
		vbl::printError(
			vbl::createSemaphores(m_renderFinishedSemaphore.data(), m_renderFinishedSemaphore.size(), m_device),
			"Failed to create semaphores"
		);
	}

	uint32_t Renderer::createRenderPass(const std::vector<VkAttachmentDescription>& attachments,
		const std::vector<VkSubpassDescription>& subpasses,
		const std::vector<VkSubpassDependency>& dependencies)
	{

		RenderPass renderPass;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = attachments.size();
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = subpasses.size();
		renderPassInfo.pSubpasses = subpasses.data();
		renderPassInfo.dependencyCount = dependencies.size();
		renderPassInfo.pDependencies = dependencies.data();

		renderPass.colorAttachmentCount.resize(subpasses.size());
		renderPass.depthAttachment.resize(subpasses.size());
		for (uint32_t i = 0; i < (uint32_t)subpasses.size(); i++) {
			renderPass.colorAttachmentCount[i] = subpasses[i].colorAttachmentCount;
			renderPass.depthAttachment[i] = subpasses[i].pDepthStencilAttachment != nullptr;
		}

		VkResult res = vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &renderPass.renderPass);
		if (res != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Renderpass");
		}

		renderPass.subpassCount = subpasses.size();

		m_renderPasses.emplace_back(renderPass);
		return m_renderPasses.size() - 1;
	}

	VkAttachmentDescription Renderer::getSwapchainAttachment() const {
		return getResolveAttachment(m_swapChain.format);
	}

	uint32_t Renderer::createFramebuffer(uint32_t renderPass, const std::vector<TexturePtr>& additionalAttachments) {
		Framebuffer framebuffer;
		framebuffer.framebuffers.resize(m_swapChain.images.size());
		for (uint32_t i = 0; i < (uint32_t)m_swapChain.images.size(); i++) {
			std::vector<VkImageView> views(additionalAttachments.size() + 1);
			views[0] = m_swapChain.imageViews[i];
			for (uint32_t j = 0; j < (uint32_t)additionalAttachments.size(); j++) {
				views[j + 1] = additionalAttachments[j]->view;
			}

			framebuffer.framebuffers[i] = createFramebuffer(m_swapChain.extent, m_renderPasses[renderPass].renderPass, views);
		}

		m_framebuffers.push_back(framebuffer);
		return m_framebuffers.size() - 1;
	}

	uint32_t Renderer::createGraphicsPipeline(uint32_t renderPass, uint32_t subpassIndex,
		const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
		const std::vector<VkPushConstantRange>& pushConstantRanges,
		const std::vector<VkPipelineShaderStageCreateInfo> shaderStages,
		VkPipelineVertexInputStateCreateInfo vertexInput
		)
	{
		Pipeline pipeline;
		vbl::printError(
			vbl::createPipelineLayout(&pipeline.layout, m_device, descriptorSetLayouts.data(), descriptorSetLayouts.size(), pushConstantRanges.data(), pushConstantRanges.size()),
			"Failed to create pipeline layout"
		);

		vbl::printError(
			vbl::createGraphicsPipeline(
				&pipeline.pipeline,
				m_device,
				pipeline.layout,
				m_renderPasses[renderPass].renderPass,
				subpassIndex,
				m_swapChain.extent,
				shaderStages.data(),
				shaderStages.size(),
				vertexInput
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

	TexturePtr Renderer::createDepthImage(VkExtent2D extent) {
		return m_pResourceManager->createDepthImage(extent);
	}

	TexturePtr Renderer::createTexture2D(const Image& image) {
		return createTexture2D(image.getExtent(), image.getPixels(), image.getChannelCount());
	}

	TexturePtr Renderer::createTexture2D(VkExtent2D extent, unsigned char* pixels, int channels) {
		auto image = m_pResourceManager->createShaderReadOnlyColorImage2D(extent);

		BufferPtr buffer = m_pResourceManager->createBuffer(image->extent.width * image->extent.height * channels, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, pixels);

		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.pNext = nullptr;
		info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		info.pInheritanceInfo = nullptr;
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

		VkBufferImageCopy copy = {};
		copy.imageExtent = image->extent;
		copy.imageOffset = { 0, 0, 0 };
		copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.imageSubresource.baseArrayLayer = 0;
		copy.imageSubresource.layerCount = 1;
		copy.imageSubresource.mipLevel = 0;

		copy.bufferImageHeight = 0;
		copy.bufferOffset = 0;
		copy.bufferRowLength = 0;

		vkCmdCopyBufferToImage(commandBuffer, buffer->buffer, image->image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, &copy);

		vkEndCommandBuffer(commandBuffer);


		TransferCommand transferCommand = {};
		transferCommand.commandBuffer = commandBuffer;
		transferCommand.srcBuffer = buffer;
		transferCommand.dstImage = image;
		transferCommand.type = TransferCommand::Type::BUFFER_TO_IMAGE;
		m_transferCommandQueue.push_back(transferCommand);

		return image;
	}

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

	BufferPtr Renderer::createVertexBuffer(VkDeviceSize size, void* initialData) {
		return m_pResourceManager->createBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, initialData);
	}

	BufferPtr Renderer::createIndexBuffer(VkDeviceSize size, void* initialData) {
		return m_pResourceManager->createBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, initialData);
	}

	BufferPtr Renderer::createUniformBuffer(VkDeviceSize size, void* initialData) {
		return m_pResourceManager->createBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, initialData);
	}

	BufferPtr Renderer::createStorageBuffer(VkDeviceSize size, void* initialData) {
		return m_pResourceManager->createBuffer(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, initialData);
	}

	ShaderPtr Renderer::createShader(const char* path, VkShaderStageFlagBits stage) {
		return std::make_shared<Shader>(m_device, path, stage);
	}

	uint32_t Renderer::createPipeline(const ShaderSet& shaderSet, uint32_t renderPass, uint32_t subpassIndex) {
		if (shaderSet.isGraphicsSet()) {

			auto vertexAttributes = shaderSet.getVertexAttributes();
			auto vertexBindings = shaderSet.getVertexBindings();

			VkPipelineVertexInputStateCreateInfo inputState = {};
			inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			inputState.pNext = nullptr;
			inputState.flags = 0;
			inputState.vertexAttributeDescriptionCount = vertexAttributes.size();
			inputState.pVertexAttributeDescriptions = vertexAttributes.data();
			inputState.vertexBindingDescriptionCount = vertexBindings.size();
			inputState.pVertexBindingDescriptions = vertexBindings.data();

			return createGraphicsPipeline(
				renderPass,
				subpassIndex,
				shaderSet.getDescriptorSetLayouts(),
				shaderSet.getPushConstantRanges(),
				shaderSet.getShaderInfos(),
				inputState
			);
		}
		else {
			//Compute pipeline
			return createComputePipeline(shaderSet.getDescriptorSetLayouts(), shaderSet.getPushConstantRanges(), shaderSet.getShaderInfos()[0]);
		}

	}

	ShaderSet Renderer::createShaderSet(const ShaderPtr& vertexShader, const ShaderPtr& fragmentShader) {
		return ShaderSet(m_device, m_swapChain.images.size(), vertexShader, fragmentShader);
	}

	ShaderSet Renderer::createShaderSet(const ShaderPtr& vertexShader, const ShaderPtr& geometryShader, const ShaderPtr& fragmentShader) {
		return ShaderSet(m_device, m_swapChain.images.size(), vertexShader, geometryShader, fragmentShader);
	}

	ShaderSet Renderer::createShaderSet(const ShaderPtr& computeShader) {
		return ShaderSet(m_device, m_swapChain.images.size(), computeShader);
	}


	CommandBufferPtr Renderer::createCommandBuffer(bool isCompute) {
		CommandBufferPtr commandBuffer = std::make_shared<CommandBuffer>();

		commandBuffer->buffers.resize(m_swapChain.images.size());
		vbl::allocateCommandBuffers(
			commandBuffer->buffers.data(),
			commandBuffer->buffers.size(),
			m_device, 
			(isCompute) ? m_computeCommandPool : m_graphicsCommandPool,
			(isCompute)? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY);

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

	void Renderer::updateDescriptorSet(const DescriptorSetPtr& descriptorSet, uint32_t binding, const BufferPtr& buffer, const TexturePtr& image, const SamplerPtr& sampler, bool isOneTimeUpdate) {

		if (descriptorSet->writes.size() <= binding) {
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
		
		std::vector<VkClearValue> clearValues;

		for (uint32_t i = 0; i < m_renderPasses[renderPass].subpassCount; i++) {
			for (uint32_t j = 0; j < m_renderPasses[renderPass].colorAttachmentCount[i]; j++) {
				VkClearValue clearValue;
				clearValue.color = { clearColor.r, clearColor.g, clearColor.b, 1.0f };
				clearValues.push_back(clearValue);
			}
			if (m_renderPasses[renderPass].depthAttachment[i]) {
				VkClearValue clearValue;
				clearValue.depthStencil.depth = 1.0f;
				clearValue.depthStencil.stencil = 0;		
				clearValues.push_back(clearValue);
			}
		}

		beginInfo.clearValueCount = clearValues.size();
		beginInfo.pClearValues = clearValues.data();
		beginInfo.framebuffer = m_framebuffers[framebuffer].framebuffers[m_frameIndex];

		VkRect2D renderArea = {};
		renderArea.extent = m_swapChain.extent;
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

	void Renderer::bindVertexBuffer(const BufferPtr& vertexBuffer, const CommandBufferPtr& commandBuffer, uint32_t frameIndex) {
		VkDeviceSize offset = 0;
		uint32_t realFrameIndex = (frameIndex == -1) ? m_frameIndex : frameIndex;
		vkCmdBindVertexBuffers((commandBuffer == nullptr)? m_graphicsCommandBuffers[realFrameIndex] : commandBuffer->buffers[realFrameIndex], 0, 1, &vertexBuffer->buffer, &offset);
	}

	void Renderer::bindVertexBuffers(const std::vector<BufferPtr>& vertexBuffers, const CommandBufferPtr& commandBuffer, uint32_t frameIndex) {

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

	void Renderer::bindIndexBuffer(const BufferPtr& indexbuffer, const CommandBufferPtr& commandBuffer, uint32_t frameIndex) {
		uint32_t realFrameIndex = (frameIndex == -1) ? m_frameIndex : frameIndex;
		vkCmdBindIndexBuffer((commandBuffer == nullptr) ? m_graphicsCommandBuffers[realFrameIndex] : commandBuffer->buffers[realFrameIndex], indexbuffer->buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void Renderer::bindDescriptorSet(const DescriptorSetPtr& descriptorSet, uint32_t pipeline, const CommandBufferPtr& commandBuffer, uint32_t frameIndex) {
		uint32_t realFrameIndex = (frameIndex == -1) ? m_frameIndex : frameIndex;
		if (m_pipelines[pipeline].isCompute) {
			vkCmdBindDescriptorSets((commandBuffer == nullptr) ? m_computeCommandBuffers[realFrameIndex] : commandBuffer->buffers[realFrameIndex],
				VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelines[pipeline].layout, 0, 1, &descriptorSet->descriptorSets[realFrameIndex], 0, nullptr);
		}
		else {
			vkCmdBindDescriptorSets((commandBuffer == nullptr) ? m_graphicsCommandBuffers[realFrameIndex] : commandBuffer->buffers[realFrameIndex],
				VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelines[pipeline].layout, 0, 1, &descriptorSet->descriptorSets[realFrameIndex], 0, nullptr);
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