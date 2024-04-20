#include "pch.h"
#include "Renderer.hpp"

#include "internal/VulkanCore.hpp"
#include "internal/debugFunctions.hpp"


#include "internal/Swapchain.hpp"
#include "internal/RenderProgram.hpp"
#include "internal/FramebufferSet.hpp"

#include "internal/DescriptorSet.hpp"

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace sa {


	PipelineConfig toConfig(PipelineSettings userSettings) {
		PipelineConfig config = {};
		config.input.topology = (vk::PrimitiveTopology)userSettings.topology;
		config.rasterizer.cullMode = (vk::CullModeFlags)userSettings.cullMode;
		config.rasterizer.polygonMode = (vk::PolygonMode)userSettings.polygonMode;
		config.depthStencil.depthTestEnable = userSettings.depthTestEnabled;
		config.tessellation.pathControlPoints = userSettings.tessellationPathControllPoints;

		config.dynamicStates.resize(userSettings.dynamicStates.size());
		memcpy(config.dynamicStates.data(), userSettings.dynamicStates.data(), userSettings.dynamicStates.size() * sizeof(vk::DynamicState));

		return std::move(config);
	}

	Renderer::Renderer() {
		try {
			vk::ApplicationInfo info = {
				.pApplicationName = "Saturn App",
				.applicationVersion = 0,
				.pEngineName = "Saturn",
				.engineVersion = 0,
				.apiVersion = SA_VK_API_VERSION
			}; 

			m_pCore = std::make_unique<VulkanCore>();
			
			m_pCore->init(info, c_useVaildationLayers);
			
			ResourceManager::Get().setCleanupFunction<Swapchain>([](Swapchain* p) { p->destroy(); });
			ResourceManager::Get().setCleanupFunction<FramebufferSet>([](FramebufferSet* p) { p->destroy(); });
			ResourceManager::Get().setCleanupFunction<RenderProgram>([](RenderProgram* p) { p->destroy(); });
			ResourceManager::Get().setCleanupFunction<vk::Pipeline>([&](vk::Pipeline* p) { m_pCore->getDevice().destroyPipeline(*p); });
			ResourceManager::Get().setCleanupFunction<vk::PipelineLayout>([&](vk::PipelineLayout* p) { m_pCore->getDevice().destroyPipelineLayout(*p); });
			ResourceManager::Get().setCleanupFunction<vk::ShaderModule>([&](vk::ShaderModule* p) { m_pCore->getDevice().destroyShaderModule(*p); });
			ResourceManager::Get().setCleanupFunction<vk::DescriptorSetLayout>([&](vk::DescriptorSetLayout* p) { m_pCore->getDevice().destroyDescriptorSetLayout(*p); });
			ResourceManager::Get().setCleanupFunction<vk::DescriptorPool>([&](vk::DescriptorPool* p) { m_pCore->getDevice().destroyDescriptorPool(*p); });
			ResourceManager::Get().setCleanupFunction<DescriptorSet>([](DescriptorSet* p) { p->destroy(); });
			ResourceManager::Get().setCleanupFunction<vk::Sampler>([&](vk::Sampler* p) { m_pCore->getDevice().destroySampler(*p); });
			ResourceManager::Get().setCleanupFunction<vk::ImageView>([&](vk::ImageView* p) { m_pCore->getDevice().destroyImageView(*p); });
			ResourceManager::Get().setCleanupFunction<vk::BufferView>([&](vk::BufferView* p) { m_pCore->getDevice().destroyBufferView(*p); });
			ResourceManager::Get().setCleanupFunction<CommandPool>([](CommandPool* p) { p->destroy(); });


		}
		catch (const std::exception& e) {
			SA_DEBUG_LOG_ERROR(e.what());
		}
	}

	Renderer& Renderer::Get() {
		static Renderer instance;
		return instance;
	}

	Renderer::~Renderer() {
		m_pCore->getDevice().waitIdle();

#ifndef IMGUI_DISABLE
		if (ImGui::GetCurrentContext() != NULL) {
			cleanupImGui();
		}
#endif
		
		ResourceManager::Get().clearContainer<CommandPool>();
		ResourceManager::Get().clearContainer<vk::BufferView>();
		ResourceManager::Get().clearContainer<vk::ImageView>();
		ResourceManager::Get().clearContainer<vk::Sampler>();
		ResourceManager::Get().clearContainer<DescriptorSet>();
		ResourceManager::Get().clearContainer<vk::DescriptorPool>();
		ResourceManager::Get().clearContainer<vk::DescriptorSetLayout>();
		ResourceManager::Get().clearContainer<vk::ShaderModule>();
		ResourceManager::Get().clearContainer<vk::Pipeline>();
		ResourceManager::Get().clearContainer<vk::PipelineLayout>();
		ResourceManager::Get().clearContainer<RenderProgram>();
		ResourceManager::Get().clearContainer<FramebufferSet>();
		ResourceManager::Get().clearContainer<Swapchain>();

		m_pCore->cleanup();
	}

	VulkanCore* Renderer::getCore() const {
		return m_pCore.get();
	}

#ifndef IMGUI_DISABLE
	void Renderer::initImGui(const Window& window, ResourceID renderProgram, uint32_t subpass) {
		RenderProgram* pRenderProgram = RenderContext::GetRenderProgram(renderProgram);
		m_pCore->initImGui(window.getWindowHandle(), pRenderProgram->getRenderPass(), subpass);
	}
	
	void Renderer::newImGuiFrame() {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void Renderer::cleanupImGui() {
		m_pCore->cleanupImGui();
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void Renderer::imGuiImage(sa::Texture texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col) {
		vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if (texture.getUsageFlags() & TextureUsageFlagBits::STORAGE) {
			layout = vk::ImageLayout::eGeneral;
		}
		VkDescriptorSet descSet = m_pCore->getImGuiImageDescriptoSet(*texture.getView(), layout);
		ImGui::Image(descSet, size, uv0, uv1, tint_col, border_col);
	}

	bool Renderer::imGuiImageButton(sa::Texture texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col) {
		VkDescriptorSet descSet = m_pCore->getImGuiImageDescriptoSet(*texture.getView(), ((DeviceImage*)texture)->layout);
		return ImGui::ImageButton(descSet, size, uv0, uv1, frame_padding, bg_col, tint_col);
	}

	ImTextureID Renderer::getImGuiTexture(const sa::Texture* texture) {
		return m_pCore->getImGuiImageDescriptoSet(*texture->getView(), vk::ImageLayout::eShaderReadOnlyOptimal);
	}

#endif

	ResourceID Renderer::createSwapchain(GLFWwindow* pWindow) {
		return ResourceManager::Get().insert<Swapchain>(m_pCore.get(), pWindow);
	}

	ResourceID Renderer::recreateSwapchain(GLFWwindow* pWindow, ResourceID oldSwapchain) {
		m_pCore->getDevice().waitIdle();
		destroySwapchain(oldSwapchain);
		return createSwapchain(pWindow);
	}

	void Renderer::destroySwapchain(ResourceID id) {
		ResourceManager::Get().remove<Swapchain>(id);
	}

	uint32_t Renderer::getSwapchainImageCount(ResourceID swapchain) {
		Swapchain* pSwapchain = RenderContext::GetSwapchain(swapchain);
		return pSwapchain->getImageCount();
	}

	RenderProgramFactory Renderer::createRenderProgram() {
		return RenderProgramFactory(m_pCore.get());
	}

	void Renderer::destroyRenderProgram(ResourceID renderProgram) {
		ResourceManager::Get().remove<RenderProgram>(renderProgram);
	}


	void Renderer::setClearColor(ResourceID renderProgram, Color color, uint32_t attachmentIndex) {
		RenderProgram* pRenderProgram = RenderContext::GetRenderProgram(renderProgram);
		pRenderProgram->setClearColor(attachmentIndex, color);
	}

	void Renderer::setClearColor(ResourceID renderProgram, Color color) {
		RenderProgram* pRenderProgram = RenderContext::GetRenderProgram(renderProgram);
		pRenderProgram->setClearColor(color);
	}

	ResourceID Renderer::createFramebuffer(ResourceID renderProgram, const DynamicTexture* pAttachmentTextures, uint32_t attachmentCount, uint32_t layers) {
		if (attachmentCount == 0 || pAttachmentTextures == nullptr)
			throw std::runtime_error("At least one attachmnet is required to create a framebuffer");

		return createFramebuffer(renderProgram, pAttachmentTextures, attachmentCount, pAttachmentTextures[0].getExtent(), layers);
	}

	ResourceID Renderer::createFramebuffer(ResourceID renderProgram, const DynamicTexture* pAttachmentTextures, uint32_t attachmentCount, Extent extent, uint32_t layers) {
		if (attachmentCount == 0 || pAttachmentTextures == nullptr)
			throw std::runtime_error("At least one attachmnet is required to create a framebuffer");

		RenderProgram* pRenderProgram = RenderContext::GetRenderProgram(renderProgram);

		return ResourceManager::Get().insert<FramebufferSet>(
			m_pCore.get(),
			pRenderProgram->getRenderPass(),
			pAttachmentTextures,
			attachmentCount,
			extent,
			layers);
	}


	ResourceID Renderer::createSwapchainFramebuffer(ResourceID renderProgram, ResourceID swapchain, const DynamicTexture* pAttachmentTextures, uint32_t attachmentCount, uint32_t layers) {
		Swapchain* pSwapchain = RenderContext::GetSwapchain(swapchain);
		RenderProgram* pRenderProgram = RenderContext::GetRenderProgram(renderProgram);

		return ResourceManager::Get().insert<FramebufferSet>(
			m_pCore.get(),
			pRenderProgram->getRenderPass(),
			pSwapchain,
			pAttachmentTextures,
			attachmentCount,
			layers);
	}


	ResourceID Renderer::createFramebuffer(ResourceID renderProgram, const Texture* pAttachmentTextures, uint32_t attachmentCount, uint32_t layers) {
		if (attachmentCount == 0 || pAttachmentTextures == nullptr)
			throw std::runtime_error("At least one attachmnet is required to create a framebuffer");

		return createFramebuffer(renderProgram, pAttachmentTextures, attachmentCount, pAttachmentTextures[0].getExtent(), layers);
	}

	ResourceID Renderer::createFramebuffer(ResourceID renderProgram, const Texture* pAttachmentTextures, uint32_t attachmentCount, Extent extent, uint32_t layers) {
		if (attachmentCount == 0 || pAttachmentTextures == nullptr)
			throw std::runtime_error("At least one attachmnet is required to create a framebuffer");

		RenderProgram* pRenderProgram = RenderContext::GetRenderProgram(renderProgram);

		return ResourceManager::Get().insert<FramebufferSet>(
			m_pCore.get(),
			pRenderProgram->getRenderPass(),
			pAttachmentTextures,
			attachmentCount,
			extent,
			layers);
	}

	ResourceID Renderer::createSwapchainFramebuffer(ResourceID renderProgram, ResourceID swapchain, const Texture* pAttachmentTextures, uint32_t attachmentCount, uint32_t layers) {
		Swapchain* pSwapchain = RenderContext::GetSwapchain(swapchain);
		RenderProgram* pRenderProgram = RenderContext::GetRenderProgram(renderProgram);

		return ResourceManager::Get().insert<FramebufferSet>(
			m_pCore.get(),
			pRenderProgram->getRenderPass(),
			pSwapchain,
			pAttachmentTextures,
			attachmentCount,
			layers);
	}

	ResourceID Renderer::createSwapchainFramebuffer(ResourceID renderProgram, ResourceID swapchain, uint32_t layers) {
		Swapchain* pSwapchain = RenderContext::GetSwapchain(swapchain);
		RenderProgram* pRenderProgram = RenderContext::GetRenderProgram(renderProgram);
		return ResourceManager::Get().insert<FramebufferSet>(
			m_pCore.get(),
			pRenderProgram->getRenderPass(),
			pSwapchain,
			(Texture*)nullptr,
			0,
			layers);
	}

	void Renderer::destroyFramebuffer(ResourceID framebuffer) {
		ResourceManager::Get().remove<FramebufferSet>(framebuffer);
	}

	Texture Renderer::getFramebufferTexture(ResourceID framebuffer, uint32_t attachmentIndex) const {
		return RenderContext::GetFramebufferSet(framebuffer)->getTexture(attachmentIndex);
	}
	
	DynamicTexture Renderer::getFramebufferDynamicTexture(ResourceID framebuffer, uint32_t attachmentIndex) const {
		return RenderContext::GetFramebufferSet(framebuffer)->getDynamicTexture(attachmentIndex);
	}

	DynamicTexture* Renderer::getFramebufferDynamicTexturePtr(ResourceID framebuffer, uint32_t attachmentIndex) const {
		return RenderContext::GetFramebufferSet(framebuffer)->getDynamicTexturePtr(attachmentIndex);
	}
	
	void Renderer::waitForFrame(ResourceID swapchains) {
		RenderContext::GetSwapchain(swapchains)->waitForFrame();
	}

	size_t Renderer::getFramebufferTextureCount(ResourceID framebuffer) const {
		return RenderContext::GetFramebufferSet(framebuffer)->getTextureCount();
	}

	Extent Renderer::getFramebufferExtent(ResourceID framebuffer) const {
		return RenderContext::GetFramebufferSet(framebuffer)->getExtent();
	}

	/*
	ResourceID Renderer::createGraphicsPipeline(ResourceID renderProgram, uint32_t subpassIndex, Extent extent, const ShaderSet& shaderSet, PipelineSettings settings) {
		RenderProgram* pRenderProgram = RenderContext::GetRenderProgram(renderProgram);
		PipelineConfig config = toConfig(settings);
		return pRenderProgram->createPipeline(shaderSet, subpassIndex, extent, config);
	}

	*/
	ResourceID Renderer::createComputePipeline(const Shader& shader, const PipelineLayout& layout) {
		vk::PipelineShaderStageCreateInfo vk_shaderStageInfo;
		
		ShaderStageInfo stageInfo = shader.getShaderStageInfo();
		const vk::ShaderModule* pModule = ResourceManager::Get().get<vk::ShaderModule>(shader.getShaderModuleID());
		if (!pModule)
			throw std::runtime_error("Invalid ShaderModule ID");

		vk_shaderStageInfo.module = *pModule;
		vk_shaderStageInfo.pName = stageInfo.pName;
		vk_shaderStageInfo.stage = static_cast<vk::ShaderStageFlagBits>(stageInfo.stage);
		vk_shaderStageInfo.pSpecializationInfo = nullptr;


		const vk::PipelineLayout* pLayout = RenderContext::GetPipelineLayout(layout.getLayoutID());

		vk::ComputePipelineCreateInfo createInfo = {};
		createInfo.layout = *pLayout;
		createInfo.stage = vk_shaderStageInfo;
		createInfo.basePipelineHandle = VK_NULL_HANDLE;
		createInfo.basePipelineIndex = 0;

		const auto pipeline = m_pCore->getDevice().createComputePipeline(VK_NULL_HANDLE, createInfo);
		return ResourceManager::Get().insert(pipeline.value);
	}

	ResourceID Renderer::createGraphicsPipeline(PipelineLayout& layout, Shader* pShaders, uint32_t shaderCount, ResourceID renderProgram, uint32_t subpassIndex, Extent extent, PipelineSettings settings) {

		std::vector<vk::PipelineShaderStageCreateInfo> vk_shaderStageInfos(shaderCount);
		for (uint32_t i = 0; i < shaderCount; i++) {
			ShaderStageInfo stageInfo = pShaders[i].getShaderStageInfo();
			const vk::ShaderModule* pModule = ResourceManager::Get().get<vk::ShaderModule>(pShaders[i].getShaderModuleID());
			if (!pModule)
				throw std::runtime_error("Invalid ShaderModule ID");
			vk_shaderStageInfos[i].module = *pModule;
			vk_shaderStageInfos[i].pName = stageInfo.pName;
			vk_shaderStageInfos[i].stage = static_cast<vk::ShaderStageFlagBits>(stageInfo.stage);
			vk_shaderStageInfos[i].pSpecializationInfo = nullptr;
		}

		const vk::PipelineLayout* pLayout = RenderContext::GetPipelineLayout(layout.getLayoutID());
		
		auto& vertexAttributes = layout.getVertexAttributes();
		std::vector<vk::VertexInputAttributeDescription> vk_vertexAttributes(vertexAttributes.size());
		for (int i = 0; i < vk_vertexAttributes.size(); i++) {
			vk_vertexAttributes[i].binding = vertexAttributes[i].binding;
			vk_vertexAttributes[i].format = static_cast<vk::Format>(vertexAttributes[i].format);
			vk_vertexAttributes[i].location = vertexAttributes[i].location;
			vk_vertexAttributes[i].offset = vertexAttributes[i].offset;
		}


		auto& vertexBindings = layout.getVertexBindings();
		std::vector<vk::VertexInputBindingDescription> vk_vertexBindings(vertexBindings.size());
		for (int i = 0; i < vk_vertexBindings.size(); i++) {
			vk_vertexBindings[i].binding = vertexBindings[i].binding;
			vk_vertexBindings[i].stride = vertexBindings[i].stride;
			vk_vertexBindings[i].inputRate = vk::VertexInputRate::eVertex;
		}

		vk::PipelineVertexInputStateCreateInfo vertexInput;
		vertexInput
			.setVertexAttributeDescriptions(vk_vertexAttributes)
			.setVertexBindingDescriptions(vk_vertexBindings);

		PipelineConfig config = toConfig(settings);

		RenderProgram* pRenderProgram = ResourceManager::Get().get<RenderProgram>(renderProgram);
		config.multisample = {
			.sampleShadingEnable = true,
			.minSampleShading = 0.2f,
			.sampleCount = pRenderProgram->getSubpass(subpassIndex).getSampleCount(),
		};

		config.colorBlends.resize(pRenderProgram->getSubpass(subpassIndex).getColorAttachments().size());

		vk::Pipeline vkPipeline = m_pCore->createGraphicsPipeline(
			*pLayout,
			pRenderProgram->getRenderPass(),
			subpassIndex,
			{ extent.width, extent.height },
			vk_shaderStageInfos,
			vertexInput,
			nullptr,
			config
		);
		return ResourceManager::Get().insert(vkPipeline);
	}

	void Renderer::destroyPipeline(ResourceID pipeline) {
		ResourceManager::Get().remove<vk::Pipeline>(pipeline);
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Buffer& buffer) {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);
		const DeviceBuffer* pDeviceBuffer = (const DeviceBuffer*)buffer;
		vk::BufferView* pView = nullptr;
		if (buffer.getType() == BufferType::UNIFORM_TEXEL|| buffer.getType() == BufferType::STORAGE_TEXEL) {
			pView = buffer.getView();
		}
		pDescriptorSet->update(binding, pDeviceBuffer->buffer, pDeviceBuffer->size, 0, pView, UINT32_MAX);
	}
	
	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, DynamicBuffer& buffer) {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);
		for (uint32_t i = 0; i < buffer.getBufferCount(); i++) {
			auto& b = buffer.getBuffer(i);
			const DeviceBuffer* pDeviceBuffer = (const DeviceBuffer*)b;
			vk::BufferView* pView = nullptr;
			if (buffer.getType() == BufferType::UNIFORM_TEXEL || buffer.getType() == BufferType::STORAGE_TEXEL) {
				pView = b.getView();
			}
			pDescriptorSet->update(binding, pDeviceBuffer->buffer, pDeviceBuffer->size, 0, pView, i);
		}
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture& texture, ResourceID sampler) {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);
		vk::Sampler* pSampler = RenderContext::GetSampler(sampler);
		vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if ((texture.getUsageFlags() & sa::TextureUsageFlagBits::STORAGE) == sa::TextureUsageFlagBits::STORAGE) {
			layout = vk::ImageLayout::eGeneral;
		}

		pDescriptorSet->update(binding, *texture.getView(), layout, pSampler, UINT32_MAX);
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture& texture) {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);
		vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if ((texture.getUsageFlags() & sa::TextureUsageFlagBits::STORAGE) == sa::TextureUsageFlagBits::STORAGE) {
			layout = vk::ImageLayout::eGeneral;
		}

		pDescriptorSet->update(binding, *texture.getView(), layout, nullptr, UINT32_MAX);
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const DynamicTexture& texture, ResourceID sampler) {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);
		vk::Sampler* pSampler = RenderContext::GetSampler(sampler);

		vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if ((texture.getUsageFlags() & sa::TextureUsageFlagBits::STORAGE) == sa::TextureUsageFlagBits::STORAGE) {
			layout = vk::ImageLayout::eGeneral;
		}

		for (uint32_t i = 0; i < texture.getTextureCount(); i++) {
			pDescriptorSet->update(binding, *texture.getTexture(i).getView(), layout, pSampler, i);
		}
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const DynamicTexture& texture) {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);

		vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if ((texture.getUsageFlags() & sa::TextureUsageFlagBits::STORAGE) == sa::TextureUsageFlagBits::STORAGE) {
			layout = vk::ImageLayout::eGeneral;
		}

		for (uint32_t i = 0; i < texture.getTextureCount(); i++) {
			pDescriptorSet->update(binding, *texture.getTexture(i).getView(), layout, nullptr, i);
		}
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const std::vector<Texture>& textures, uint32_t firstElement) {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);
		pDescriptorSet->update(binding, firstElement, textures.data(), textures.size(), nullptr, UINT32_MAX);
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const std::vector<Texture>& textures, ResourceID sampler, uint32_t firstElement) {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);
		vk::Sampler* pSampler = RenderContext::GetSampler(sampler);
		pDescriptorSet->update(binding, firstElement, textures.data(), textures.size(), pSampler, UINT32_MAX);
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture* textures, uint32_t textureCount, ResourceID sampler, uint32_t firstElement) {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);
		vk::Sampler* pSampler = RenderContext::GetSampler(sampler);
		pDescriptorSet->update(binding, firstElement, textures, textureCount, pSampler, UINT32_MAX);
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture* textures, uint32_t textureCount, uint32_t firstElement) {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);
		pDescriptorSet->update(binding, firstElement, textures, textureCount, nullptr, UINT32_MAX);
	}


	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, ResourceID sampler) {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);
		vk::Sampler* pSampler = RenderContext::GetSampler(sampler);
		pDescriptorSet->update(binding, VK_NULL_HANDLE, vk::ImageLayout::eUndefined, pSampler, UINT32_MAX);
	}

	void Renderer::freeDescriptorSet(ResourceID descriptorSet) {
		ResourceManager::Get().remove<DescriptorSet>(descriptorSet);
	}

	DeviceMemoryStats Renderer::getGPUMemoryUsage() const {
		return std::move(m_pCore->getGPUMemoryUsage());
	}
	
	DataTransfer* Renderer::queueTransfer(const DataTransfer& transfer) {
		const std::lock_guard<std::mutex> lock(m_transferMutex);
		return &m_transferQueue.emplace_back(transfer);
	}

	bool Renderer::cancelTransfer(DataTransfer* pTransfer) {
		const std::lock_guard<std::mutex> lock(m_transferMutex);
		for (auto it = m_transferQueue.begin(); it != m_transferQueue.end(); it++) {
			if (pTransfer == &(*it)) {
				m_transferQueue.erase(it);
				return true;
			}
		}
		return false;
	}

	ResourceID Renderer::createSampler(FilterMode filterMode) {
		vk::SamplerCreateInfo info{
			.magFilter = (vk::Filter)filterMode,
			.minFilter = (vk::Filter)filterMode,
			.mipmapMode = vk::SamplerMipmapMode::eNearest,
			.mipLodBias = 0.0f,
			.minLod = 0,
			.maxLod = 9,
		};
		return ResourceManager::Get().insert(m_pCore->createSampler(info));
	}

	ResourceID Renderer::createSampler(const SamplerInfo& samplerInfo) {
		vk::SamplerCreateInfo info = {};
		
		info.magFilter = (vk::Filter)samplerInfo.magFilter;
		info.minFilter = (vk::Filter)samplerInfo.minFilter;
		info.mipmapMode = (vk::SamplerMipmapMode)samplerInfo.mipmapMode;
		info.addressModeU = (vk::SamplerAddressMode)samplerInfo.addressModeU;
		info.addressModeV = (vk::SamplerAddressMode)samplerInfo.addressModeV;
		info.addressModeW = (vk::SamplerAddressMode)samplerInfo.addressModeW;
		info.mipLodBias = samplerInfo.mipLodBias;
		info.anisotropyEnable = samplerInfo.anisotropyEnable;
		info.maxAnisotropy = samplerInfo.maxAnisotropy;
		info.compareEnable = samplerInfo.compareEnable;
		info.compareOp = (vk::CompareOp)samplerInfo.compareOp;
		info.minLod = samplerInfo.minLod;
		info.maxLod = samplerInfo.maxLod;
		info.borderColor = (vk::BorderColor)samplerInfo.borderColor;
		info.unnormalizedCoordinates = samplerInfo.unnormalizedCoordinates;

		return ResourceManager::Get().insert(m_pCore->createSampler(info));
	}

	void Renderer::destroySampler(ResourceID sampler) {
		ResourceManager::Get().remove<vk::Sampler>(sampler);
	}


	RenderContext Renderer::beginFrame(ResourceID swapchain) {
		Swapchain* pSwapchain = RenderContext::GetSwapchain(swapchain);

		CommandBufferSet* pCommandBufferSet = pSwapchain->beginFrame();
		if (!pCommandBufferSet) {
			return {};
		}

		m_pCore->setMemoryManagerFrameIndex(pSwapchain->getFrameIndex());

		m_transferMutex.lock();
		while (!m_transferQueue.empty()) {
			DataTransfer& transfer = m_transferQueue.front();

			switch (transfer.type) {
			case DataTransfer::Type::BUFFER_TO_IMAGE:
			{
				vk::FormatProperties properties = m_pCore->getPhysicalDevice().getFormatProperties(transfer.dstImage->format);
				if (transfer.dstImage->mipLevels > 1 && !(properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
					SA_DEBUG_LOG_WARNING("Mipmap not supported by format", vk::to_string(transfer.dstImage->format), ". No mipmaps generated");
					transfer.dstImage->mipLevels = 1;
				}
				m_pCore->transferBufferToColorImage(
					pCommandBufferSet->getBuffer(),
					transfer.srcBuffer->buffer,
					transfer.dstImage->image,
					transfer.dstImage->mipLevels,
					transfer.dstImage->arrayLayers,
					transfer.dstImage->extent,
					transfer.dstImage->layout,
					vk::ImageLayout::eShaderReadOnlyOptimal,
					vk::AccessFlagBits::eShaderRead,
					vk::PipelineStageFlagBits::eFragmentShader);
				transfer.dstImage->layout = vk::ImageLayout::eShaderReadOnlyOptimal;
				break;
			}
			case DataTransfer::Type::BUFFER_TO_BUFFER:
			case DataTransfer::Type::IMAGE_TO_BUFFER:	
			case DataTransfer::Type::IMAGE_TO_IMAGE:
				throw std::runtime_error("unimplemented case");
				break;
			default:
				throw std::runtime_error("Invalid transfer type");
				break;
			}

			
			m_transferQueue.pop_front();
		}
		m_transferMutex.unlock();

		return RenderContext(m_pCore.get(), pCommandBufferSet);
	}

	void Renderer::endFrame(ResourceID swapchain) {
		Swapchain* pSwapchain = RenderContext::GetSwapchain(swapchain);
		pSwapchain->endFrame();
	}

	ResourceID Renderer::createContextPool() {
		ResourceID id = ResourceManager::Get().insert<CommandPool>();
		ResourceManager::Get().get<CommandPool>(id)->create(m_pCore->getDevice(), m_pCore->getQueueFamily(), vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
		return id;
	}

	DirectContext Renderer::createDirectContext(ResourceID contextPool) {
		return DirectContext(m_pCore.get(), contextPool);
	}

	SubContext Renderer::createSubContext(ResourceID framebuffer, ResourceID renderProgram, uint32_t subpassIndex, ResourceID contextPool) {
		FramebufferSet* pFramebufferSet = RenderContext::GetFramebufferSet(framebuffer);
		RenderProgram* pRenderProgram = RenderContext::GetRenderProgram(renderProgram);
		
		return SubContext(m_pCore.get(), pFramebufferSet, pRenderProgram, subpassIndex, contextPool);
	}

	SubContext Renderer::createSubContext(ResourceID contextPool) {
		return SubContext(m_pCore.get(), nullptr, nullptr, 0, contextPool);
	}

	Format Renderer::selectFormat(const std::vector<Format>& formatCandidates, TextureUsageFlags textureType) const {
		vk::FormatFeatureFlags features = (vk::FormatFeatureFlagBits)0;

		if (textureType & TextureUsageFlagBits::DEPTH_ATTACHMENT) {
			features |= vk::FormatFeatureFlagBits::eDepthStencilAttachment;
		}
		if (textureType & TextureUsageFlagBits::SAMPLED) {
			features |= vk::FormatFeatureFlagBits::eSampledImage;
		}
		if (textureType & TextureUsageFlagBits::COLOR_ATTACHMENT) {
			features |= vk::FormatFeatureFlagBits::eColorAttachment;
		}
		if (textureType & TextureUsageFlagBits::STORAGE) {
			features |= vk::FormatFeatureFlagBits::eStorageImage;
		}
		if (textureType & TextureUsageFlagBits::TRANSFER_DST) {
			features |= vk::FormatFeatureFlagBits::eTransferDst;
		}
		std::vector<vk::Format> candidates(formatCandidates.size());
		memcpy(candidates.data(), formatCandidates.data(), candidates.size() * sizeof(vk::Format));

		return (Format)m_pCore->getFormat(candidates, features, vk::ImageTiling::eOptimal);
	}

	Format Renderer::selectFormat(TextureUsageFlags textureType) const {
		return selectFormat(
			FormatPrecisionFlagBits::ANY_PRECISION, 
			FormatDimensionFlagBits::ANY_DIMENSION, 
			sa::FormatTypeFlagBits::ANY_TYPE, 
			textureType);
	}

	Format Renderer::selectFormat(FormatPrecisionFlags precisions, FormatDimensionFlags dimensions, FormatTypeFlags types, TextureUsageFlags textureType) const {
		vk::FormatFeatureFlags features = (vk::FormatFeatureFlagBits)0;

		if (textureType & TextureUsageFlagBits::DEPTH_ATTACHMENT) {
			features |= vk::FormatFeatureFlagBits::eDepthStencilAttachment;
		}
		if (textureType & TextureUsageFlagBits::SAMPLED) {
			features |= vk::FormatFeatureFlagBits::eSampledImage;
		}
		if (textureType & TextureUsageFlagBits::COLOR_ATTACHMENT) {
			features |= vk::FormatFeatureFlagBits::eColorAttachment;
		}
		if (textureType & TextureUsageFlagBits::STORAGE) {
			features |= vk::FormatFeatureFlagBits::eStorageImage;
		}
		if (textureType & TextureUsageFlagBits::TRANSFER_DST) {
			features |= vk::FormatFeatureFlagBits::eTransferDst;
		}

		return (Format)m_pCore->getFormat(
			precisions, 
			dimensions, 
			types, 
			features, 
			vk::ImageTiling::eOptimal);

	}

	Format Renderer::getDefaultDepthFormat() const {
		return (Format)m_pCore->getDefaultDepthFormat();
	}

	Format Renderer::getDefaultColorFormat() const {
		return (Format)m_pCore->getDefaultColorFormat();
	}

	Format Renderer::getAttachmentFormat(ResourceID renderProgram, uint32_t attachmentIndex) const {
		RenderProgram* pRenderProgram = RenderContext::GetRenderProgram(renderProgram);
		vk::AttachmentDescription attachment = pRenderProgram->getAttachment(attachmentIndex);
		return (Format)attachment.format;
	}


}

namespace ImGui {
	void Image(sa::Texture texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col) {
		sa::Renderer::Get().imGuiImage(texture, size, uv0, uv1, tint_col, border_col);
	}

	bool ImageButton(sa::Texture texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col) {
		return sa::Renderer::Get().imGuiImageButton(texture, size, uv0, uv1, frame_padding, bg_col, tint_col);
	}
}
