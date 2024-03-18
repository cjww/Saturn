#include "pch.h"
#include "RenderContext.hpp"

#include "internal/RenderProgram.hpp"
#include "internal/FramebufferSet.hpp"
#include "internal/Pipeline.hpp"
#include "internal/Swapchain.hpp"

#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

namespace sa {

	Swapchain* RenderContext::getSwapchain(ResourceID id) {
		Swapchain* pSwapchain = ResourceManager::get().get<Swapchain>(id);
		if (!pSwapchain)
			throw std::runtime_error("Nonexistent swapchain: " + id);
		return pSwapchain;
	}

	RenderProgram* RenderContext::getRenderProgram(ResourceID id) {
		RenderProgram* pRenderProgram = ResourceManager::get().get<RenderProgram>(id);
		if (!pRenderProgram)
			throw std::runtime_error("Nonexistent render program: " + id);
		return pRenderProgram;
	}

	FramebufferSet* RenderContext::getFramebufferSet(ResourceID id) {
		FramebufferSet* pFramebufferSet = ResourceManager::get().get<FramebufferSet>(id);
		if (!pFramebufferSet)
			throw std::runtime_error("Nonexistent framebuffer: " + id);
		return pFramebufferSet;
	}

	Pipeline* RenderContext::getPipeline(ResourceID id) {
		Pipeline* pPipeline = ResourceManager::get().get<Pipeline>(id);
		if (!pPipeline)
			throw std::runtime_error("Nonexistent pipeline: " + id);
		return pPipeline;
	}

	DescriptorSet* RenderContext::getDescriptorSet(ResourceID id) {
		DescriptorSet* pDescriptorSet = ResourceManager::get().get<DescriptorSet>(id);
		if (!pDescriptorSet)
			throw std::runtime_error("Nonexistent descriptor set: " + id);
		return pDescriptorSet;
	}

	vk::Sampler* RenderContext::getSampler(ResourceID id) {
		vk::Sampler* pSampler = ResourceManager::get().get<vk::Sampler>(id);
		if (!pSampler)
			throw std::runtime_error("Nonexistent sampler: " + id);
		return pSampler;
	}

	RenderContext::RenderContext()
		: m_pCommandBufferSet(nullptr)
		, m_boundPipeline(NULL_RESOURCE)
	{
	}

	RenderContext::RenderContext(VulkanCore* pCore, CommandBufferSet* pCommandBufferSet)
		: m_pCommandBufferSet(pCommandBufferSet)
		, m_pCore(pCore)
	{
	
	}

#ifndef IMGUI_DISABLE
	void RenderContext::renderImGuiFrame() {
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_pCommandBufferSet->getBuffer());

		// Update and Render additional Platform Windows
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}
#endif
	void RenderContext::beginRenderProgram(ResourceID renderProgram, ResourceID framebuffer, SubpassContents contents, Rect renderArea) {
		RenderProgram* pRenderProgram = getRenderProgram(renderProgram);
		FramebufferSet* pFramebuffer = getFramebufferSet(framebuffer);
		pRenderProgram->begin(m_pCommandBufferSet, pFramebuffer, (vk::SubpassContents)contents, renderArea);
	}

	void RenderContext::nextSubpass(SubpassContents contentType) {
		m_pCommandBufferSet->getBuffer().nextSubpass((vk::SubpassContents)contentType);
	}

	void RenderContext::endRenderProgram(ResourceID renderProgram) {
		RenderProgram* pRenderProgram = getRenderProgram(renderProgram);
		pRenderProgram->end(m_pCommandBufferSet);
	}

	void RenderContext::executeSubContext(const sa::SubContext& context) {
		m_pCommandBufferSet->getBuffer().executeCommands(context.m_pCommandBufferSet->getBuffer(m_pCommandBufferSet->getBufferIndex()));
	}

	void RenderContext::bindPipeline(ResourceID pipeline) {
		m_boundPipeline = pipeline;
		Pipeline* pPipeline = getPipeline(pipeline);
		pPipeline->bind(m_pCommandBufferSet);
	}

	void RenderContext::bindVertexBuffers(uint32_t firstBinding, const std::vector<Buffer>& buffers) {
		if (buffers.empty())
			return;

		std::vector<vk::Buffer> vkBuffers;
		std::vector<vk::DeviceSize> offsets(buffers.size(), 0);
		vkBuffers.reserve(buffers.size());
		for (const Buffer& buffer : buffers) {
			if (buffer.getType() != BufferType::VERTEX) {
				SA_DEBUG_LOG_ERROR("All buffers must be of VERTEX type");
				return;
			}
			const DeviceBuffer* deviceBuffer = (const DeviceBuffer*)buffer;
			vkBuffers.push_back(deviceBuffer->buffer);
		}
		m_pCommandBufferSet->getBuffer().bindVertexBuffers(firstBinding, vkBuffers, offsets);
	}

	void RenderContext::bindIndexBuffer(const Buffer& buffer) {
		const DeviceBuffer* deviceBuffer = (const DeviceBuffer*)buffer;
		m_pCommandBufferSet->getBuffer().bindIndexBuffer(deviceBuffer->buffer, 0, vk::IndexType::eUint32);
	}

	void RenderContext::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Buffer& buffer) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		const DeviceBuffer* pDeviceBuffer = (const DeviceBuffer*)buffer;
		vk::BufferView* pView = nullptr;
		if (buffer.getType() == BufferType::UNIFORM_TEXEL || buffer.getType() == BufferType::STORAGE_TEXEL) {
			pView = buffer.getView();
		}
		pDescriptorSet->update(binding, pDeviceBuffer->buffer, pDeviceBuffer->size, 0, pView, m_pCommandBufferSet->getBufferIndex());
	}

	void RenderContext::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture& texture, ResourceID sampler) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		vk::Sampler* pSampler = RenderContext::getSampler(sampler);
		vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if ((texture.getTypeFlags() & sa::TextureTypeFlagBits::STORAGE) == sa::TextureTypeFlagBits::STORAGE) {
			layout = vk::ImageLayout::eGeneral;
		}

		pDescriptorSet->update(binding, *texture.getView(), layout, pSampler, m_pCommandBufferSet->getBufferIndex());
	}

	void RenderContext::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture& texture) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if ((texture.getTypeFlags() & sa::TextureTypeFlagBits::STORAGE) == sa::TextureTypeFlagBits::STORAGE) {
			layout = vk::ImageLayout::eGeneral;
		}

		pDescriptorSet->update(binding, *texture.getView(), layout, nullptr, m_pCommandBufferSet->getBufferIndex());
	}

	void RenderContext::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const std::vector<Texture>& textures, uint32_t firstElement) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		pDescriptorSet->update(binding, firstElement, textures.data(), textures.size(), nullptr, m_pCommandBufferSet->getBufferIndex());
	}

	void RenderContext::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const std::vector<Texture>& textures, ResourceID sampler, uint32_t firstElement) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		vk::Sampler* pSampler = RenderContext::getSampler(sampler);
		pDescriptorSet->update(binding, firstElement, textures.data(), textures.size(), pSampler, m_pCommandBufferSet->getBufferIndex());
	}

	void RenderContext::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture* textures, uint32_t textureCount, ResourceID sampler, uint32_t firstElement) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		vk::Sampler* pSampler = RenderContext::getSampler(sampler);
		pDescriptorSet->update(binding, firstElement, textures, textureCount, pSampler, m_pCommandBufferSet->getBufferIndex());
	}

	void RenderContext::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture* textures, uint32_t textureCount, uint32_t firstElement) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		pDescriptorSet->update(binding, firstElement, textures, textureCount, nullptr, m_pCommandBufferSet->getBufferIndex());
	}

	void RenderContext::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, ResourceID sampler) {
		DescriptorSet* pDescriptorSet = RenderContext::getDescriptorSet(descriptorSet);
		vk::Sampler* pSampler = RenderContext::getSampler(sampler);
		pDescriptorSet->update(binding, VK_NULL_HANDLE, vk::ImageLayout::eUndefined, pSampler, m_pCommandBufferSet->getBufferIndex());
	}

	void RenderContext::bindDescriptorSet(ResourceID descriptorSet) {
		Pipeline* pPipeline = getPipeline(m_boundPipeline);
		DescriptorSet* pDescriptorSet = getDescriptorSet(descriptorSet);
		pPipeline->bindDescriptorSet(m_pCommandBufferSet, pDescriptorSet);
	}

	void RenderContext::pushConstants(ShaderStageFlags stages, uint32_t offset, uint32_t size, void* data) {
		Pipeline* pPipeline = getPipeline(m_boundPipeline);
		if (offset != UINT32_MAX) {
			pPipeline->pushConstants(m_pCommandBufferSet, (vk::ShaderStageFlags)stages, offset, size, data);
		}
		else {
			pPipeline->pushConstants(m_pCommandBufferSet, (vk::ShaderStageFlags)stages, size, data);
		}
	}

	void RenderContext::bindDescriptorSets(const std::vector<ResourceID>& descriptorSets) {
		Pipeline* pPipeline = getPipeline(m_boundPipeline);
		std::vector<vk::DescriptorSet> sets;
		sets.reserve(descriptorSets.size());
		uint32_t firstSet = UINT32_MAX;
		for (auto id : descriptorSets) {
			DescriptorSet* pDescriptorSet = getDescriptorSet(id);
			if (firstSet == UINT32_MAX)
				firstSet = pDescriptorSet->getSetIndex();
			sets.push_back(pDescriptorSet->getSet(m_pCommandBufferSet->getBufferIndex()));
		}

		pPipeline->bindDescriptorSets(m_pCommandBufferSet, firstSet, sets);
	}

	void RenderContext::setScissor(Rect scissor) {
		m_pCommandBufferSet->getBuffer().setScissor(0, vk::Rect2D{ { scissor.offset.x, scissor.offset.y }, { scissor.extent.width, scissor.extent.height } });
	}

	void RenderContext::setViewport(Rect viewport) {
		vk::Viewport vp = {};
		vp.width = viewport.extent.width;
		vp.height = viewport.extent.height;
		vp.maxDepth = 1.0f;
		vp.minDepth = 0.0f;
		vp.x = viewport.offset.x;
		vp.y = viewport.offset.y;
		m_pCommandBufferSet->getBuffer().setViewport(0, vp);
	}


	void RenderContext::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
		m_pCommandBufferSet->getBuffer().draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void RenderContext::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) {
		m_pCommandBufferSet->getBuffer().drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void RenderContext::drawIndexedIndirect(const Buffer& buffer, uint32_t offset, uint32_t drawCount, uint32_t stride) {
		const DeviceBuffer* pBuffer = (const DeviceBuffer*)buffer;
		m_pCommandBufferSet->getBuffer().drawIndexedIndirect(pBuffer->buffer, offset, drawCount, stride);
	}

	void RenderContext::drawIndexedIndirect(const Buffer& buffer, size_t offset, const Buffer& countBuffer, size_t countOffset, uint32_t stride, uint32_t maxDrawCount) {
		const DeviceBuffer* pBuffer = (const DeviceBuffer*)buffer;
		const DeviceBuffer* pCountBuffer = (const DeviceBuffer*)countBuffer;

		m_pCommandBufferSet->getBuffer().drawIndexedIndirectCount(pBuffer->buffer, offset, pCountBuffer->buffer, countOffset, maxDrawCount, stride);
	}

	void RenderContext::drawIndirect(const Buffer& buffer, uint32_t offset, uint32_t drawCount, uint32_t stride) {
		const DeviceBuffer* pBuffer = (const DeviceBuffer*)buffer;
		m_pCommandBufferSet->getBuffer().drawIndirect(pBuffer->buffer, offset, drawCount, stride);
	}

	void RenderContext::drawIndirect(const Buffer& buffer, size_t offset, const Buffer& countBuffer, size_t countOffset, uint32_t stride, uint32_t maxDrawCount) {
		const DeviceBuffer* pBuffer = (const DeviceBuffer*)buffer;
		const DeviceBuffer* pCountBuffer = (const DeviceBuffer*)countBuffer;
		m_pCommandBufferSet->getBuffer().drawIndirectCount(pBuffer->buffer, offset, pCountBuffer->buffer, countOffset, maxDrawCount, stride);
	}

	void RenderContext::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
		m_pCommandBufferSet->getBuffer().dispatch(groupCountX, groupCountY, groupCountZ);
	}

	void RenderContext::barrierColorAttachment(const Texture& texture) {

		DeviceImage* pImage = (DeviceImage*)texture;
		vk::ImageLayout newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if (texture.getTypeFlags() & sa::TextureTypeFlagBits::STORAGE)
			newLayout = vk::ImageLayout::eGeneral;


		vk::ImageMemoryBarrier imageBarrier{
			.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
			.dstAccessMask = vk::AccessFlagBits::eShaderRead,
			.oldLayout = pImage->layout,
			.newLayout = newLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = pImage->image,
			.subresourceRange{
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.baseMipLevel = 0,
				.levelCount = pImage->mipLevels,
				.baseArrayLayer = 0,
				.layerCount = pImage->arrayLayers,
			},
		};

		pImage->layout = newLayout;

		m_pCommandBufferSet->getBuffer().pipelineBarrier(
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eFragmentShader,
			(vk::DependencyFlags)0,
			nullptr,
			nullptr,
			imageBarrier);
	}

	void RenderContext::barrierColorCompute(const Texture& texture) {

		DeviceImage* pImage = (DeviceImage*)texture;
		vk::ImageLayout newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if (texture.getTypeFlags() & sa::TextureTypeFlagBits::STORAGE)
			newLayout = vk::ImageLayout::eGeneral;

		vk::ImageMemoryBarrier imageBarrier{
			.srcAccessMask = vk::AccessFlagBits::eShaderWrite,
			.dstAccessMask = vk::AccessFlagBits::eShaderRead,
			.oldLayout = pImage->layout,
			.newLayout = newLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = pImage->image,
			.subresourceRange{
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.baseMipLevel = 0,
				.levelCount = pImage->mipLevels,
				.baseArrayLayer = 0,
				.layerCount = pImage->arrayLayers,
			},
		};

		pImage->layout = newLayout;

		m_pCommandBufferSet->getBuffer().pipelineBarrier(
			vk::PipelineStageFlagBits::eComputeShader,
			vk::PipelineStageFlagBits::eFragmentShader,
			(vk::DependencyFlags)0,
			nullptr,
			nullptr,
			imageBarrier);
	}

	void RenderContext::barrierColorCompute(const Buffer& buffer) {

		const DeviceBuffer* pBuffer = (const DeviceBuffer*)buffer;
		
		vk::BufferMemoryBarrier bufferBarrier {
			.srcAccessMask = vk::AccessFlagBits::eShaderWrite,
			.dstAccessMask = vk::AccessFlagBits::eShaderRead,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.buffer = pBuffer->buffer,
			.offset = 0,
			.size = pBuffer->size,
		};

		m_pCommandBufferSet->getBuffer().pipelineBarrier(
			vk::PipelineStageFlagBits::eComputeShader,
			vk::PipelineStageFlagBits::eFragmentShader,
			(vk::DependencyFlags)0,
			nullptr,
			bufferBarrier,
			nullptr);
	}

	void RenderContext::transitionTexture(const Texture& texture, Transition src, Transition dst) {

		vk::AccessFlags srcAccess;
		vk::AccessFlags dstAccess;
		vk::PipelineStageFlags srcStage;
		vk::PipelineStageFlags dstStage;
		vk::ImageLayout newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor;

		switch (src) {
		case sa::Transition::NONE:
			srcAccess = (vk::AccessFlags)0;
			srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
			break;
		case sa::Transition::RENDER_PROGRAM_INPUT:
			srcAccess = vk::AccessFlagBits::eInputAttachmentRead;
			srcStage = vk::PipelineStageFlagBits::eFragmentShader;
			break;
		case sa::Transition::RENDER_PROGRAM_OUTPUT:
			srcAccess = vk::AccessFlagBits::eColorAttachmentWrite;
			srcStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			break;
		case sa::Transition::RENDER_PROGRAM_DEPTH_OUTPUT:
			srcAccess = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			srcStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
			aspectFlags = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
			break;
		case sa::Transition::COMPUTE_SHADER_READ:
			srcAccess = vk::AccessFlagBits::eShaderRead;
			srcStage = vk::PipelineStageFlagBits::eComputeShader;
			break;
		case sa::Transition::COMPUTE_SHADER_WRITE:
			srcAccess = vk::AccessFlagBits::eShaderWrite;
			srcStage = vk::PipelineStageFlagBits::eComputeShader;
			break;
		case sa::Transition::FRAGMENT_SHADER_READ:
			dstAccess = vk::AccessFlagBits::eShaderRead;
			dstStage = vk::PipelineStageFlagBits::eFragmentShader;
			break;
		case sa::Transition::FRAGMENT_SHADER_WRITE:
			dstAccess = vk::AccessFlagBits::eShaderWrite;
			dstStage = vk::PipelineStageFlagBits::eFragmentShader;
			break;
		default:
			break;
		}

		switch (dst) {
		case sa::Transition::NONE:
			dstAccess = (vk::AccessFlags)0;
			dstStage = vk::PipelineStageFlagBits::eTopOfPipe;
			break;
		case sa::Transition::RENDER_PROGRAM_INPUT:
			dstAccess = vk::AccessFlagBits::eInputAttachmentRead;
			dstStage = vk::PipelineStageFlagBits::eFragmentShader;
			break;
		case sa::Transition::RENDER_PROGRAM_OUTPUT:
			dstAccess = vk::AccessFlagBits::eColorAttachmentWrite;
			dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			newLayout = vk::ImageLayout::eColorAttachmentOptimal;
			break;
		case sa::Transition::RENDER_PROGRAM_DEPTH_OUTPUT:
			dstAccess = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
			dstStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
			newLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			aspectFlags = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
			break;
		case sa::Transition::COMPUTE_SHADER_READ:
			dstAccess = vk::AccessFlagBits::eShaderRead;
			dstStage = vk::PipelineStageFlagBits::eComputeShader;
			if (texture.getTypeFlags() & TextureTypeFlagBits::STORAGE) {
				newLayout = vk::ImageLayout::eGeneral;
			}
			break;
		case sa::Transition::COMPUTE_SHADER_WRITE:
			dstAccess = vk::AccessFlagBits::eShaderWrite;
			dstStage = vk::PipelineStageFlagBits::eComputeShader;
			if (texture.getTypeFlags() & TextureTypeFlagBits::STORAGE) {
				newLayout = vk::ImageLayout::eGeneral;
			}
			break;
		case sa::Transition::FRAGMENT_SHADER_READ:
			dstAccess = vk::AccessFlagBits::eShaderRead;
			dstStage = vk::PipelineStageFlagBits::eFragmentShader;
			if (texture.getTypeFlags() & TextureTypeFlagBits::STORAGE) {
				newLayout = vk::ImageLayout::eGeneral;
			}
			break;
		case sa::Transition::FRAGMENT_SHADER_WRITE:
			dstAccess = vk::AccessFlagBits::eShaderWrite;
			dstStage = vk::PipelineStageFlagBits::eFragmentShader;
			if (texture.getTypeFlags() & TextureTypeFlagBits::STORAGE) {
				newLayout = vk::ImageLayout::eGeneral;
			}
			break;

		default:
			break;
		}

		DeviceImage* pImage = (DeviceImage*)texture;
		m_pCore->transferImageLayout(
			m_pCommandBufferSet->getBuffer(),
			pImage->layout,
			newLayout,
			srcAccess,
			dstAccess,
			pImage->image,
			aspectFlags,
			pImage->mipLevels,
			pImage->arrayLayers,
			srcStage,
			dstStage
		);
		pImage->layout = newLayout;
	}

	void RenderContext::copyImageToImageColor(const Texture& src, const Texture& dst) {
		const DeviceImage* pSrc = src;
		const DeviceImage* pDst = dst;

		vk::ImageCopy region = {
			.srcSubresource = {
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 0,
			},
			.srcOffset = { 0, 0, 0 },
			.dstSubresource = {
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 0,
			},
			.dstOffset = { 0, 0, 0 },
			.extent = pSrc->extent
		};

		m_pCommandBufferSet->getBuffer().copyImage(pSrc->image, pSrc->layout, pDst->image, pDst->layout, region);
	}
	
	void RenderContext::copyImageToSwapchain(const Texture& src, ResourceID swapchain) {
		const DeviceImage* pSrc = src;
		Swapchain* pSwapchain = getSwapchain(swapchain);
		vk::Image swapchainImage = pSwapchain->getImage(pSwapchain->getImageIndex());

		vk::ImageCopy region = {
			.srcSubresource = {
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 1,
			},
			.srcOffset = { 0, 0, 0 },
			.dstSubresource = {
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.mipLevel = 0,
					.baseArrayLayer = 0,
					.layerCount = 1,
			},
			.dstOffset = { 0, 0, 0 },
			.extent = pSrc->extent
		};

		m_pCommandBufferSet->getBuffer().copyImage(pSrc->image, pSrc->layout, swapchainImage, vk::ImageLayout::ePresentSrcKHR, region);
	}

	uint32_t RenderContext::getFrameIndex() const {
		return m_pCommandBufferSet->getBufferIndex();
	}


	SubContext::SubContext()
		: RenderContext()
	{
	}

	SubContext::SubContext(VulkanCore* pCore, FramebufferSet* pFramebufferSet, RenderProgram* pRenderProgram, uint32_t subpassIndex, ResourceID contextPool) {
		m_pCore = pCore;

		if (contextPool == NULL_RESOURCE) {
			m_commandBufferSetID = ResourceManager::get().insert(m_pCore->allocateCommandBufferSet(vk::CommandBufferLevel::eSecondary));
		}
		else {
			CommandPool* pool = ResourceManager::get().get<CommandPool>(contextPool);
			m_commandBufferSetID = ResourceManager::get().insert(m_pCore->allocateCommandBufferSet(vk::CommandBufferLevel::eSecondary, *pool));
		}
		m_pCommandBufferSet = ResourceManager::get().get<CommandBufferSet>(m_commandBufferSetID);

		m_pFramebufferSet = pFramebufferSet;
		m_pRenderProgram = pRenderProgram;
		m_subpassIndex = subpassIndex;
	}

	void SubContext::begin(ContextUsageFlags usageFlags) {

		vk::CommandBufferInheritanceInfo inheritInfo{
			.renderPass = m_pRenderProgram ? m_pRenderProgram->getRenderPass() : VK_NULL_HANDLE,
			.subpass = m_subpassIndex,
			.framebuffer = m_pFramebufferSet ? m_pFramebufferSet->getBuffer(m_pCommandBufferSet->getBufferIndex() % m_pFramebufferSet->getBufferCount()) : VK_NULL_HANDLE,
			.occlusionQueryEnable = VK_FALSE,
		};

		m_pCommandBufferSet->begin((vk::CommandBufferUsageFlags)usageFlags, &inheritInfo);
	}

	void SubContext::end() {
		m_pCommandBufferSet->end();
	}

	void SubContext::preRecord(std::function<void(RenderContext&)> function, ContextUsageFlags usageFlags) {
		for (uint32_t i = 0; i < m_pCommandBufferSet->getBufferCount(); i++) {
			begin(usageFlags);
			function(*this);
			end();
		}
	}

	void SubContext::destroy() {
		ResourceManager::get().remove<CommandBufferSet>(m_commandBufferSetID);
		m_pCommandBufferSet = nullptr;
	}

	DirectContext::DirectContext(VulkanCore* pCore, ResourceID contextPool) {
		m_pCore = pCore;
		
		if (contextPool == NULL_RESOURCE) {
			m_commandBufferSetID = ResourceManager::get().insert(m_pCore->allocateCommandBufferSet(vk::CommandBufferLevel::ePrimary));
		}
		else {
			CommandPool* pool = ResourceManager::get().get<CommandPool>(contextPool);
			m_commandBufferSetID = ResourceManager::get().insert(m_pCore->allocateCommandBufferSet(vk::CommandBufferLevel::ePrimary, *pool));
		}


		m_pCommandBufferSet = ResourceManager::get().get<CommandBufferSet>(m_commandBufferSetID);

		m_pFence = std::shared_ptr<vk::Fence>(new vk::Fence, [=](vk::Fence* p) {
			m_pCore->getDevice().destroyFence(*p);
			delete p;
			});
		*m_pFence = m_pCore->getDevice().createFence({ .flags = vk::FenceCreateFlagBits::eSignaled });
	}

	void DirectContext::begin(ContextUsageFlags usageFlags) {
		m_pCommandBufferSet->begin((vk::CommandBufferUsageFlags)usageFlags);
	}

	void DirectContext::end() {
		m_pCommandBufferSet->end();
	}

	void DirectContext::submit() {
		waitToFinish();
		m_pCore->getDevice().resetFences(*m_pFence);
		m_pCommandBufferSet->submit(*m_pFence);
	}

	void DirectContext::waitToFinish(size_t timeout) {
		m_pCore->getDevice().waitForFences(*m_pFence, VK_FALSE, timeout);
	}

	void DirectContext::destroy() {
		ResourceManager::get().remove<CommandBufferSet>(m_commandBufferSetID);
		m_pCommandBufferSet = nullptr;
	}


}
