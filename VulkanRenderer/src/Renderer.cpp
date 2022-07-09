#include "pch.h"
#include "Renderer.hpp"

#include "VulkanCore.hpp"
#include "debugFunctions.h"


#include "Resources/Swapchain.hpp"
#include "Resources\RenderProgram.hpp"
#include "Resources/FramebufferSet.hpp"
#include "Resources/Pipeline.hpp"
#include "Resources/DescriptorSet.hpp"

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace sa {


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
			
			m_pCore->init(info);

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
			DEBUG_LOG_ERROR(e.what());
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
			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
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

	void Renderer::imGuiImage(sa::Texture texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col) {
		VkDescriptorSet descSet = m_pCore->getImGuiImageDescriptoSet(texture.getView(), ((DeviceImage*)texture)->layout);
		ImGui::Image(descSet, size, uv0, uv1, tint_col, border_col);
	}

	bool Renderer::imGuiImageButton(sa::Texture texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col) {
		VkDescriptorSet descSet = m_pCore->getImGuiImageDescriptoSet(texture.getView(), ((DeviceImage*)texture)->layout);
		return ImGui::ImageButton(descSet, size, uv0, uv1, frame_padding, bg_col, tint_col);
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

	void Renderer::setClearColor(ResourceID renderProgram, Color color, uint32_t attachmentIndex) {
		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);
		pRenderProgram->setClearColor(attachmentIndex, color);
	}

	void Renderer::setClearColor(ResourceID renderProgram, Color color) {
		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);
		pRenderProgram->setClearColor(color);
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

	ResourceID Renderer::createGraphicsPipeline(ResourceID renderProgram, uint32_t subpassIndex, Extent extent, const std::string& vertexShader, const std::string& fragmentShader) {
		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);
		Shader vShader(m_pCore->getDevice(), vertexShader.c_str(), vk::ShaderStageFlagBits::eVertex);
		Shader fShader(m_pCore->getDevice(), fragmentShader.c_str(), vk::ShaderStageFlagBits::eFragment);
		ShaderSet set(m_pCore->getDevice(), vShader, fShader);

		return pRenderProgram->createPipeline(set, subpassIndex, extent);
	}
	
	ResourceID Renderer::createGraphicsPipeline(ResourceID renderProgram, uint32_t subpassIndex, Extent extent, const std::string& vertexShader, const std::string& geometryShader, const std::string& fragmentShader) {
		RenderProgram* pRenderProgram = RenderContext::getRenderProgram(renderProgram);
		Shader vShader(m_pCore->getDevice(), vertexShader.c_str(), vk::ShaderStageFlagBits::eVertex);
		Shader gShader(m_pCore->getDevice(), geometryShader.c_str(), vk::ShaderStageFlagBits::eGeometry);
		Shader fShader(m_pCore->getDevice(), fragmentShader.c_str(), vk::ShaderStageFlagBits::eFragment);
		ShaderSet set(m_pCore->getDevice(), vShader, gShader, fShader);

		return pRenderProgram->createPipeline(set, subpassIndex, extent);
	}

	ResourceID Renderer::createComputePipeline(const std::string& computeShader) {
		Shader cShader(m_pCore->getDevice(), computeShader.c_str(), vk::ShaderStageFlagBits::eCompute);
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

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture2D& texture, ResourceID sampler) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		vk::Sampler* pSampler = RenderContext::getSampler(sampler);
		pDescriptorSet->update(binding, *texture.getView(), pSampler, UINT32_MAX);
	}

	void Renderer::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture2D& texture) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		pDescriptorSet->update(binding, *texture.getView(), nullptr, UINT32_MAX);
	}

	void Renderer::freeDescriptorSet(ResourceID descriptorSet) {
		ResourceManager::get().remove<DescriptorSet>(descriptorSet);
	}

	Buffer Renderer::createBuffer(BufferType type, size_t size, void* initialData) {
		return Buffer(m_pCore.get(), type, size, initialData);
	}

	Texture2D Renderer::createTexture2D(TextureTypeFlags type, Extent extent, uint32_t sampleCount) {
		return Texture2D(m_pCore.get(), type, extent, sampleCount);
	}

	Texture2D Renderer::createTexture2D(TextureTypeFlags type, Extent extent, FormatPrecisionFlags formatPrecision, FormatDimensionFlags formatDimensions, FormatTypeFlags formatType, uint32_t sampleCount) {
		return Texture2D(m_pCore.get(), type, extent, formatPrecision, formatDimensions, formatType, sampleCount);
	}

	Texture2D Renderer::createTexture2D(TextureTypeFlags type, Extent extent, ResourceID swapchain, uint32_t sampleCount) {
		Swapchain* pSwapchain = RenderContext::getSwapchain(swapchain);
		return Texture2D(m_pCore.get(), type, extent, pSwapchain, sampleCount);
	}

	Texture2D Renderer::createTexture2D(const Image& image, bool generateMipMaps) {
		return Texture2D(m_pCore.get(), image, generateMipMaps);
	}

	void Renderer::queueTransfer(const DataTransfer& transfer) {
		m_transferQueue.push(transfer);
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

	RenderContext Renderer::beginFrame(ResourceID swapchain) {
		Swapchain* pSwapchain = RenderContext::getSwapchain(swapchain);

		CommandBufferSet* pCommandBufferSet = pSwapchain->beginFrame();
		if (!pCommandBufferSet) {
			return {};
		}

		while (!m_transferQueue.empty()) {
			DataTransfer& transfer = m_transferQueue.front();

			switch (transfer.type) {
			case DataTransfer::Type::BUFFER_TO_IMAGE:
			{
				vk::FormatProperties properties = m_pCore->getPhysicalDevice().getFormatProperties(transfer.dstImage->format);
				if (transfer.dstImage->mipLevels > 1 && !(properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
					DEBUG_LOG_WARNING("Mipmap not supported by format", vk::to_string(transfer.dstImage->format), ". No mipmaps generated");
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

			m_transferQueue.pop();
		}

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


}

namespace ImGui {
	void Image(sa::Texture texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col) {
		sa::Renderer::get().imGuiImage(texture, size, uv0, uv1, tint_col, border_col);
	}

	bool ImageButton(sa::Texture texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col) {
		return sa::Renderer::get().imGuiImageButton(texture, size, uv0, uv1, frame_padding, bg_col, tint_col);
	}
}
