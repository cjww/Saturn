#include "pch.h"
#include "Renderer.hpp"

#include "internal/VulkanCore.hpp"
#include "internal/debugFunctions.hpp"


#include "internal/Swapchain.hpp"
#include "internal/RenderProgram.hpp"
#include "internal/FramebufferSet.hpp"
#include "internal/Pipeline.hpp"
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
				.apiVersion = VK_API_VERSION_1_3
			}; 

			m_pCore = std::make_unique<VulkanCore>();
			
			m_pCore->init(info, c_useVaildationLayers);
			
			ResourceManager::get().setCleanupFunction<Swapchain>([](Swapchain* p) { p->destroy(); });
			ResourceManager::get().setCleanupFunction<FramebufferSet>([](FramebufferSet* p) { p->destroy(); });
			ResourceManager::get().setCleanupFunction<RenderProgram>([](RenderProgram* p) { p->destroy(); });
			ResourceManager::get().setCleanupFunction<Pipeline>([](Pipeline* p) { p->destroy(); });
			ResourceManager::get().setCleanupFunction<DescriptorSet>([](DescriptorSet* p) { p->destroy(); });
			ResourceManager::get().setCleanupFunction<vk::Sampler>([&](vk::Sampler* p) { m_pCore->getDevice().destroySampler(*p); });
			ResourceManager::get().setCleanupFunction<vk::ImageView>([&](vk::ImageView* p) { m_pCore->getDevice().destroyImageView(*p); });
			ResourceManager::get().setCleanupFunction<vk::BufferView>([&](vk::BufferView* p) { m_pCore->getDevice().destroyBufferView(*p); });
			ResourceManager::get().setCleanupFunction<CommandPool>([](CommandPool* p) { p->destroy(); });

		}
		catch (const std::exception& e) {
			SA_DEBUG_LOG_ERROR(e.what());
		}
	}

	Renderer& Renderer::get() {
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
		
		ResourceManager::get().clearContainer<CommandPool>();
		ResourceManager::get().clearContainer<vk::BufferView>();
		ResourceManager::get().clearContainer<vk::ImageView>();
		ResourceManager::get().clearContainer<vk::Sampler>();
		ResourceManager::get().clearContainer<DescriptorSet>();
		ResourceManager::get().clearContainer<Pipeline>();
		ResourceManager::get().clearContainer<FramebufferSet>();
		ResourceManager::get().clearContainer<RenderProgram>();
		ResourceManager::get().clearContainer<Swapchain>();

		m_pCore->cleanup();
	}

#ifndef IMGUI_DISABLE
	void Renderer::initImGui(const Window& window, ResourceID renderProgram, uint32_t subpass) {
		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);
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
		if (texture.getTypeFlags() & TextureTypeFlagBits::STORAGE) {
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
		return ResourceManager::get().insert<Swapchain>(m_pCore.get(), pWindow);
	}

	ResourceID Renderer::recreateSwapchain(GLFWwindow* pWindow, ResourceID oldSwapchain) {
		m_pCore->getDevice().waitIdle();
		destroySwapchain(oldSwapchain);
		return createSwapchain(pWindow);
	}

	void Renderer::destroySwapchain(ResourceID id) {
		ResourceManager::get().remove<Swapchain>(id);
	}

	uint32_t Renderer::getSwapchainImageCount(ResourceID swapchain) {
		Swapchain* pSwapchain = RenderContext::getSwapchain(swapchain);
		return pSwapchain->getImageCount();
	}

	RenderProgramFactory Renderer::createRenderProgram() {
		return RenderProgramFactory(m_pCore.get());
	}

	void Renderer::destroyRenderProgram(ResourceID renderProgram) {
		ResourceManager::get().remove<RenderProgram>(renderProgram);
	}


	void Renderer::setClearColor(ResourceID renderProgram, Color color, uint32_t attachmentIndex) {
		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);
		pRenderProgram->setClearColor(attachmentIndex, color);
	}

	void Renderer::setClearColor(ResourceID renderProgram, Color color) {
		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);
		pRenderProgram->setClearColor(color);
	}

	ResourceID Renderer::createFramebuffer(ResourceID renderProgram, const std::vector<DynamicTexture>& attachmentTextures, uint32_t layers) {
		if (attachmentTextures.empty())
			throw std::runtime_error("At least one attachmnet is required to create a framebuffer");

		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);
		
		return ResourceManager::get().insert<FramebufferSet>(
			m_pCore.get(),
			pRenderProgram->getRenderPass(),
			attachmentTextures,
			attachmentTextures[0].getExtent(),
			layers);
	}

	ResourceID Renderer::createSwapchainFramebuffer(ResourceID renderProgram, ResourceID swapchain, const std::vector<DynamicTexture>& additionalAttachmentTextures, uint32_t layers) {
		Swapchain* pSwapchain = RenderContext::getSwapchain(swapchain);
		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);

		return ResourceManager::get().insert<FramebufferSet>(
			m_pCore.get(),
			pRenderProgram->getRenderPass(),
			pSwapchain,
			additionalAttachmentTextures,
			layers);
	}


	ResourceID Renderer::createFramebuffer(ResourceID renderProgram, const std::vector<Texture>& attachmentTextures, uint32_t layers) {
		if (attachmentTextures.empty())
			throw std::runtime_error("At least one attachmnet is required to create a framebuffer");

		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);

		return ResourceManager::get().insert<FramebufferSet>(
			m_pCore.get(),
			pRenderProgram->getRenderPass(),
			attachmentTextures,
			attachmentTextures[0].getExtent(),
			layers);
	}

	ResourceID Renderer::createSwapchainFramebuffer(ResourceID renderProgram, ResourceID swapchain, const std::vector<Texture>& additionalAttachmentTextures, uint32_t layers) {
		Swapchain* pSwapchain = RenderContext::getSwapchain(swapchain);
		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);

		return ResourceManager::get().insert<FramebufferSet>(
			m_pCore.get(),
			pRenderProgram->getRenderPass(),
			pSwapchain,
			additionalAttachmentTextures,
			layers);
	}

	void Renderer::destroyFramebuffer(ResourceID framebuffer) {
		ResourceManager::get().remove<FramebufferSet>(framebuffer);
	}

	Texture Renderer::getFramebufferTexture(ResourceID framebuffer, uint32_t attachmentIndex) const {
		return RenderContext::getFramebufferSet(framebuffer)->getTexture(attachmentIndex);
	}
	
	DynamicTexture Renderer::getFramebufferDynamicTexture(ResourceID framebuffer, uint32_t attachmentIndex) const {
		return RenderContext::getFramebufferSet(framebuffer)->getDynamicTexture(attachmentIndex);
	}

	DynamicTexture* Renderer::getFramebufferDynamicTexturePtr(ResourceID framebuffer, uint32_t attachmentIndex) const {
		return RenderContext::getFramebufferSet(framebuffer)->getDynamicTexturePtr(attachmentIndex);
	}
	
	void Renderer::waitForFrame(ResourceID swapchains) {
		RenderContext::getSwapchain(swapchains)->waitForFrame();
	}

	size_t Renderer::getFramebufferTextureCount(ResourceID framebuffer) const {
		return RenderContext::getFramebufferSet(framebuffer)->getTextureCount();
	}

	Extent Renderer::getFramebufferExtent(ResourceID framebuffer) const {
		return RenderContext::getFramebufferSet(framebuffer)->getExtent();
	}

	void Renderer::swapFramebuffer(ResourceID framebuffer) {
		FramebufferSet* pFramebufferSet = RenderContext::getFramebufferSet(framebuffer);
		pFramebufferSet->swap();
	}

	ResourceID Renderer::createShaderModule(const std::string& shaderSpvPath, ShaderStage stage) {
		return ResourceManager::get().insert<ShaderModule>(m_pCore->getDevice(), shaderSpvPath.c_str(), (vk::ShaderStageFlagBits)stage);
	}

	ResourceID Renderer::createGraphicsPipeline(ResourceID renderProgram, uint32_t subpassIndex, Extent extent, const std::vector<ResourceID>& shaderModules, PipelineSettings settings) {
		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);

		std::vector<ShaderModule> shaders;
		shaders.reserve(shaderModules.size());
		for (auto& id : shaderModules) {
			ShaderModule* pShaderModule = RenderContext::getShaderModule(id);
			shaders.push_back(*pShaderModule);
		}
		ShaderSet set(m_pCore->getDevice(), shaders);

		PipelineConfig config = toConfig(settings);

		return pRenderProgram->createPipeline(set, subpassIndex, extent, config);
	}

	ResourceID Renderer::createComputePipeline(const std::string& computeShader) {
		ShaderModule cShader(m_pCore->getDevice(), computeShader.c_str(), vk::ShaderStageFlagBits::eCompute);
		ShaderSet set(m_pCore->getDevice(), cShader);
		PipelineConfig config = {};
		return ResourceManager::get().insert<Pipeline>(m_pCore.get(), set, config);
	}

	void Renderer::destroyPipeline(ResourceID pipeline) {
		ResourceManager::get().remove<Pipeline>(pipeline);
	}

	ResourceID Renderer::allocateDescriptorSet(ResourceID pipeline, uint32_t setIndex) {
		Pipeline* pPipeline = RenderContext::getPipeline(pipeline);
		return ResourceManager::get().insert<DescriptorSet>(pPipeline->allocateDescriptSet(setIndex));
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Buffer& buffer) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		const DeviceBuffer* pDeviceBuffer = (const DeviceBuffer*)buffer;
		vk::BufferView* pView = nullptr;
		if (buffer.getType() == BufferType::UNIFORM_TEXEL|| buffer.getType() == BufferType::STORAGE_TEXEL) {
			pView = buffer.getView();
		}
		pDescriptorSet->update(binding, pDeviceBuffer->buffer, pDeviceBuffer->size, 0, pView, UINT32_MAX);
	}
	
	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, DynamicBuffer& buffer) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
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
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		vk::Sampler* pSampler = RenderContext::getSampler(sampler);
		vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if ((texture.getTypeFlags() & sa::TextureTypeFlagBits::STORAGE) == sa::TextureTypeFlagBits::STORAGE) {
			layout = vk::ImageLayout::eGeneral;
		}

		pDescriptorSet->update(binding, *texture.getView(), layout, pSampler, UINT32_MAX);
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture& texture) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if ((texture.getTypeFlags() & sa::TextureTypeFlagBits::STORAGE) == sa::TextureTypeFlagBits::STORAGE) {
			layout = vk::ImageLayout::eGeneral;
		}

		pDescriptorSet->update(binding, *texture.getView(), layout, nullptr, UINT32_MAX);
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const DynamicTexture& texture, ResourceID sampler) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		vk::Sampler* pSampler = RenderContext::getSampler(sampler);

		vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if ((texture.getTypeFlags() & sa::TextureTypeFlagBits::STORAGE) == sa::TextureTypeFlagBits::STORAGE) {
			layout = vk::ImageLayout::eGeneral;
		}

		for (uint32_t i = 0; i < texture.getTextureCount(); i++) {
			pDescriptorSet->update(binding, *texture.getTexture(i).getView(), layout, pSampler, i);
		}
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const DynamicTexture& texture) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);

		vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if ((texture.getTypeFlags() & sa::TextureTypeFlagBits::STORAGE) == sa::TextureTypeFlagBits::STORAGE) {
			layout = vk::ImageLayout::eGeneral;
		}

		for (uint32_t i = 0; i < texture.getTextureCount(); i++) {
			pDescriptorSet->update(binding, *texture.getTexture(i).getView(), layout, nullptr, i);
		}
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const std::vector<Texture>& textures, uint32_t firstElement) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		pDescriptorSet->update(binding, firstElement, textures, nullptr, UINT32_MAX);
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, ResourceID sampler) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		vk::Sampler* pSampler = RenderContext::getSampler(sampler);
		pDescriptorSet->update(binding, VK_NULL_HANDLE, vk::ImageLayout::eUndefined, pSampler, UINT32_MAX);
	}

	void Renderer::freeDescriptorSet(ResourceID descriptorSet) {
		ResourceManager::get().remove<DescriptorSet>(descriptorSet);
	}

	Buffer Renderer::createBuffer(BufferType type, size_t size, void* initialData) {
		return Buffer(m_pCore.get(), type, size, initialData);
	}

	DynamicBuffer Renderer::createDynamicBuffer(BufferType type, size_t size, void* initialData) {
		return DynamicBuffer(m_pCore.get(), type, m_pCore->getQueueCount(), size, initialData);
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
		return ResourceManager::get().insert(m_pCore->createSampler(info));
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

		return ResourceManager::get().insert(m_pCore->createSampler(info));
	}

	RenderContext Renderer::beginFrame(ResourceID swapchain) {
		Swapchain* pSwapchain = RenderContext::getSwapchain(swapchain);

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
		Swapchain* pSwapchain = RenderContext::getSwapchain(swapchain);
		pSwapchain->endFrame();
	}

	ResourceID Renderer::createContextPool() {
		ResourceID id = ResourceManager::get().insert<CommandPool>();
		ResourceManager::get().get<CommandPool>(id)->create(m_pCore->getDevice(), m_pCore->getQueueFamily(), vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
		return id;
	}

	DirectContext Renderer::createDirectContext(ResourceID contextPool) {
		return DirectContext(m_pCore.get(), contextPool);
	}

	SubContext Renderer::createSubContext(ResourceID framebuffer, ResourceID renderProgram, uint32_t subpassIndex, ResourceID contextPool) {
		FramebufferSet* pFramebufferSet = RenderContext::getFramebufferSet(framebuffer);
		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);
		
		return SubContext(m_pCore.get(), pFramebufferSet, pRenderProgram, subpassIndex, contextPool);
	}

	SubContext Renderer::createSubContext(ResourceID contextPool) {
		return SubContext(m_pCore.get(), nullptr, nullptr, 0, contextPool);
	}

	Format Renderer::selectFormat(const std::vector<Format>& formatCandidates, TextureTypeFlags textureType) const {
		vk::FormatFeatureFlags features = (vk::FormatFeatureFlagBits)0;

		if (textureType & TextureTypeFlagBits::DEPTH_ATTACHMENT) {
			features |= vk::FormatFeatureFlagBits::eDepthStencilAttachment;
		}
		if (textureType & TextureTypeFlagBits::SAMPLED) {
			features |= vk::FormatFeatureFlagBits::eSampledImage;
		}
		if (textureType & TextureTypeFlagBits::COLOR_ATTACHMENT) {
			features |= vk::FormatFeatureFlagBits::eColorAttachment;
		}
		if (textureType & TextureTypeFlagBits::STORAGE) {
			features |= vk::FormatFeatureFlagBits::eStorageImage;
		}
		if (textureType & TextureTypeFlagBits::TRANSFER_DST) {
			features |= vk::FormatFeatureFlagBits::eTransferDst;
		}
		std::vector<vk::Format> candidates(formatCandidates.size());
		memcpy(candidates.data(), formatCandidates.data(), candidates.size() * sizeof(vk::Format));

		return (Format)m_pCore->getFormat(candidates, features, vk::ImageTiling::eOptimal);
	}

	Format Renderer::getAttachmentFormat(ResourceID renderProgram, uint32_t attachmentIndex) const {
		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);
		vk::AttachmentDescription attachment = pRenderProgram->getAttachment(attachmentIndex);
		return (Format)attachment.format;
	}


}

namespace ImGui {
	void Image(sa::Texture texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col) {
		sa::Renderer::get().imGuiImage(texture, size, uv0, uv1, tint_col, border_col);
	}

	bool ImageButton(sa::Texture texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col) {
		return sa::Renderer::get().imGuiImageButton(texture, size, uv0, uv1, frame_padding, bg_col, tint_col);
	}
}
