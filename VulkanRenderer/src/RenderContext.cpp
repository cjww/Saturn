#include "pch.h"
#include "RenderContext.hpp"

#include "internal/RenderProgram.hpp"
#include "internal/FramebufferSet.hpp"
#include "internal/Swapchain.hpp"
#include "internal/DescriptorSet.hpp"

#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

namespace sa {

	Swapchain* RenderContext::GetSwapchain(ResourceID id) {
		Swapchain* pSwapchain = ResourceManager::Get().get<Swapchain>(id);
		if (!pSwapchain)
			throw std::runtime_error("Nonexistent swapchain: " + id);
		return pSwapchain;
	}

	RenderProgram* RenderContext::GetRenderProgram(ResourceID id) {
		RenderProgram* pRenderProgram = ResourceManager::Get().get<RenderProgram>(id);
		if (!pRenderProgram)
			throw std::runtime_error("Nonexistent render program: " + id);
		return pRenderProgram;
	}

	FramebufferSet* RenderContext::GetFramebufferSet(ResourceID id) {
		FramebufferSet* pFramebufferSet = ResourceManager::Get().get<FramebufferSet>(id);
		if (!pFramebufferSet)
			throw std::runtime_error("Nonexistent framebuffer: " + id);
		return pFramebufferSet;
	}

	vk::Pipeline* RenderContext::GetPipeline(ResourceID id) {
		vk::Pipeline* pPipeline = ResourceManager::Get().get<vk::Pipeline>(id);
		if (!pPipeline)
			throw std::runtime_error("Nonexistent pipeline: " + id);
		return pPipeline;
	}

	DescriptorSet* RenderContext::GetDescriptorSet(ResourceID id) {
		DescriptorSet* pDescriptorSet = ResourceManager::Get().get<DescriptorSet>(id);
		if (!pDescriptorSet)
			throw std::runtime_error("Nonexistent descriptor set: " + id);
		return pDescriptorSet;
	}

	vk::Sampler* RenderContext::GetSampler(ResourceID id) {
		vk::Sampler* pSampler = ResourceManager::Get().get<vk::Sampler>(id);
		if (!pSampler)
			throw std::runtime_error("Nonexistent sampler: " + id);
		return pSampler;
	}

	vk::PipelineLayout* RenderContext::GetPipelineLayout(ResourceID id) {
		vk::PipelineLayout* pLayout = ResourceManager::Get().get<vk::PipelineLayout>(id);
		if (!pLayout)
			throw std::runtime_error("Nonexistent pipeline layout: " + id);
		return pLayout;
	}

	void RenderContext::bindVertexInput(const PipelineLayout& layout) const {
		const auto& vertexInputs = m_pLastPipelineLayout->getVertexBindings();

		//TODO: inefficient to allocate memory every Call
		std::vector<VkVertexInputBindingDescription2EXT> bindingDesc;
		bindingDesc.reserve(vertexInputs.size());
		for (const auto& input : vertexInputs) {
			vk::VertexInputBindingDescription2EXT binding = {};
			binding.binding = input.binding;
			binding.inputRate = vk::VertexInputRate::eVertex;
			binding.divisor = 1;
			binding.stride = input.stride;
			bindingDesc.push_back(binding);
		}

		const auto& vertexAttributes = m_pLastPipelineLayout->getVertexAttributes();
		std::vector<VkVertexInputAttributeDescription2EXT> attribDesc;
		attribDesc.reserve(vertexAttributes.size());
		for (const auto& input : vertexAttributes) {
			vk::VertexInputAttributeDescription2EXT attribute = {};
			attribute.binding = input.binding;
			attribute.offset = input.offset;
			attribute.format = static_cast<vk::Format>(input.format);
			attribute.location = input.location;
			attribDesc.push_back(attribute);
		}

		//m_pCommandBufferSet->getBuffer().setVertexInputEXT(bindingDesc, attribDesc);
		vkCmdSetVertexInputEXT(m_pCommandBufferSet->getBuffer(), bindingDesc.size(), bindingDesc.data(), attribDesc.size(), attribDesc.data());

	}

	RenderContext::RenderContext()
		: m_pCommandBufferSet(nullptr)
		, m_pCore(nullptr)
		, m_pLastPipelineLayout(nullptr)
	{
	}

	RenderContext::RenderContext(VulkanCore* pCore, CommandBufferSet* pCommandBufferSet)
		: m_pCommandBufferSet(pCommandBufferSet)
		, m_pCore(pCore)
		, m_pLastPipelineLayout(nullptr)
	{
	
	}

#ifndef IMGUI_DISABLE
	void RenderContext::renderImGuiFrame() const {
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_pCommandBufferSet->getBuffer());

		// Update and Render additional Platform Windows
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}
#endif
	void RenderContext::beginRenderProgram(ResourceID renderProgram, ResourceID framebuffer, SubpassContents contents, Rect renderArea) const {
		RenderProgram* pRenderProgram = GetRenderProgram(renderProgram);
		FramebufferSet* pFramebuffer = GetFramebufferSet(framebuffer);
		pRenderProgram->begin(m_pCommandBufferSet, pFramebuffer, (vk::SubpassContents)contents, renderArea);
	}

	
	void RenderContext::beginRendering(const std::vector<Texture>& colorAttachments, const std::vector<Texture>& depthAttachments) {
		throw std::runtime_error("Not implemented");
		/*
		vk::RenderingAttachmentInfo attachmentInfo = {};
		attachmentInfo.

		vk::RenderingInfo info = {};
		info.

		m_pCommandBufferSet->getBuffer().beginRendering();
		*/
	}

	void RenderContext::nextSubpass(SubpassContents contentType) const {
		m_pCommandBufferSet->getBuffer().nextSubpass((vk::SubpassContents)contentType);
	}

	void RenderContext::endRenderProgram(ResourceID renderProgram) const {
		RenderProgram* pRenderProgram = GetRenderProgram(renderProgram);
		pRenderProgram->end(m_pCommandBufferSet);
	}

	void RenderContext::executeSubContext(const sa::SubContext& context) const {
		m_pCommandBufferSet->getBuffer().executeCommands(context.m_pCommandBufferSet->getBuffer(m_pCommandBufferSet->getBufferIndex()));
	}

	void RenderContext::bindPipelineLayout(const PipelineLayout& pipelineLayout) {
		m_pLastPipelineLayout = const_cast<PipelineLayout*>(&pipelineLayout);
	}

	void RenderContext::bindPipeline(ResourceID pipeline) const {
		vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eCompute;
		if (m_pLastPipelineLayout->isGraphicsPipeline())
			bindPoint = vk::PipelineBindPoint::eGraphics;

		vk::Pipeline* pPipeline = GetPipeline(pipeline);
		m_pCommandBufferSet->getBuffer().bindPipeline(bindPoint, *pPipeline);
	}

	void RenderContext::bindShader(const Shader& shader) const {
		const VkShaderEXT* pShader = ResourceManager::Get().get<VkShaderEXT>(shader.getShaderObjectID());
		if (!pShader)
			throw std::runtime_error("Nonexistent shader: " + shader.getShaderObjectID());

		const VkShaderStageFlagBits stage = static_cast<VkShaderStageFlagBits>(shader.getStage());
		vkCmdBindShadersEXT(m_pCommandBufferSet->getBuffer(), 1, &stage, pShader);

		if(shader.getStage() == sa::ShaderStageFlagBits::VERTEX) {
			bindVertexInput(*m_pLastPipelineLayout);
		}

	}

	void RenderContext::bindShaders(const std::vector<Shader>& shaders) const {
		std::array<VkShaderEXT, 5> vkShaders = { VK_NULL_HANDLE };
		std::array<VkShaderStageFlagBits, 5> stages = {
			VK_SHADER_STAGE_VERTEX_BIT,
			VK_SHADER_STAGE_GEOMETRY_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
			VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
		};
		
		for (size_t i = 0; i < stages.size(); i++) {
			for(size_t j = 0; j < shaders.size(); j++) {
				if(shaders[j].getStage() == stages[i]) {
					const VkShaderEXT* pShader = ResourceManager::Get().get<VkShaderEXT>(shaders[j].getShaderObjectID());
					vkShaders[i] = pShader ? *pShader : VK_NULL_HANDLE;
				}
			}
		}

		vkCmdBindShadersEXT(m_pCommandBufferSet->getBuffer(), stages.size(), stages.data(), vkShaders.data());

		if (vkShaders[0] != VK_NULL_HANDLE) {
			bindVertexInput(*m_pLastPipelineLayout);
		}

	}

	void RenderContext::bindVertexBuffers(uint32_t firstBinding, const Buffer* pBuffers, size_t bufferCount) const {
		if (bufferCount == 0)
			return;

		std::vector<vk::Buffer> vkBuffers;
		std::vector<vk::DeviceSize> offsets(bufferCount, 0);
		vkBuffers.reserve(bufferCount);
		for (size_t i = 0; i < bufferCount; ++i) {
			const Buffer& buffer = pBuffers[i];
			if (buffer.getType() != BufferType::VERTEX) {
				SA_DEBUG_LOG_ERROR("All buffers must be of VERTEX type");
				return;
			}
			const DeviceBuffer* deviceBuffer = (const DeviceBuffer*)buffer;
			vkBuffers.push_back(deviceBuffer->buffer);
		}
		m_pCommandBufferSet->getBuffer().bindVertexBuffers(firstBinding, vkBuffers, offsets);
	}

	void RenderContext::bindIndexBuffer(const Buffer& buffer) const {
		const DeviceBuffer* deviceBuffer = (const DeviceBuffer*)buffer;
		m_pCommandBufferSet->getBuffer().bindIndexBuffer(deviceBuffer->buffer, 0, vk::IndexType::eUint32);
	}

	void RenderContext::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Buffer& buffer) const {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);
		const DeviceBuffer* pDeviceBuffer = (const DeviceBuffer*)buffer;
		vk::BufferView* pView = nullptr;
		if (buffer.getType() == BufferType::UNIFORM_TEXEL || buffer.getType() == BufferType::STORAGE_TEXEL) {
			pView = buffer.getView();
		}
		pDescriptorSet->update(binding, pDeviceBuffer->buffer, pDeviceBuffer->size, 0, pView, m_pCommandBufferSet->getBufferIndex());
	}

	void RenderContext::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture& texture, ResourceID sampler) const {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);
		vk::Sampler* pSampler = RenderContext::GetSampler(sampler);
		vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if ((texture.getUsageFlags() & sa::TextureUsageFlagBits::STORAGE) == sa::TextureUsageFlagBits::STORAGE) {
			layout = vk::ImageLayout::eGeneral;
		}

		pDescriptorSet->update(binding, *texture.getView(), layout, pSampler, m_pCommandBufferSet->getBufferIndex());
	}

	void RenderContext::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture& texture) const {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);
		vk::ImageLayout layout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if ((texture.getUsageFlags() & sa::TextureUsageFlagBits::STORAGE) == sa::TextureUsageFlagBits::STORAGE) {
			layout = vk::ImageLayout::eGeneral;
		}

		pDescriptorSet->update(binding, *texture.getView(), layout, nullptr, m_pCommandBufferSet->getBufferIndex());
	}

	void RenderContext::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const std::vector<Texture>& textures, uint32_t firstElement) const {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);
		pDescriptorSet->update(binding, firstElement, textures.data(), textures.size(), nullptr, m_pCommandBufferSet->getBufferIndex());
	}

	void RenderContext::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const std::vector<Texture>& textures, ResourceID sampler, uint32_t firstElement) const {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);
		vk::Sampler* pSampler = RenderContext::GetSampler(sampler);
		pDescriptorSet->update(binding, firstElement, textures.data(), textures.size(), pSampler, m_pCommandBufferSet->getBufferIndex());
	}

	void RenderContext::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture* textures, uint32_t textureCount, ResourceID sampler, uint32_t firstElement) const {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);
		vk::Sampler* pSampler = RenderContext::GetSampler(sampler);
		pDescriptorSet->update(binding, firstElement, textures, textureCount, pSampler, m_pCommandBufferSet->getBufferIndex());
	}

	void RenderContext::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, const Texture* textures, uint32_t textureCount, uint32_t firstElement) const {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);
		pDescriptorSet->update(binding, firstElement, textures, textureCount, nullptr, m_pCommandBufferSet->getBufferIndex());
	}

	void RenderContext::updateDescriptorSet(ResourceID descriptorSet, uint32_t binding, ResourceID sampler) const {
		DescriptorSet* pDescriptorSet = RenderContext::GetDescriptorSet(descriptorSet);
		vk::Sampler* pSampler = RenderContext::GetSampler(sampler);
		pDescriptorSet->update(binding, VK_NULL_HANDLE, vk::ImageLayout::eUndefined, pSampler, m_pCommandBufferSet->getBufferIndex());
	}

	void RenderContext::bindDescriptorSets(const std::vector<ResourceID>& descriptorSets) const {
		std::vector<vk::DescriptorSet> sets;
		sets.reserve(descriptorSets.size());
		uint32_t firstSet = UINT32_MAX;
		for (const auto id : descriptorSets) {
			const DescriptorSet* pDescriptorSet = GetDescriptorSet(id);
			if (firstSet == UINT32_MAX)
				firstSet = pDescriptorSet->getSetIndex();
			sets.push_back(pDescriptorSet->getSet(m_pCommandBufferSet->getBufferIndex()));
		}

		vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eCompute;
		if (m_pLastPipelineLayout->isGraphicsPipeline()) {
			bindPoint = vk::PipelineBindPoint::eGraphics;
		}
		vk::PipelineLayout* pLayout = GetPipelineLayout(m_pLastPipelineLayout->getLayoutID());

		m_pCommandBufferSet->getBuffer().bindDescriptorSets(bindPoint, *pLayout, firstSet, sets, nullptr);
	}

	void RenderContext::bindDescriptorSet(ResourceID descriptorSet) const {
		DescriptorSet* pDescriptorSet = GetDescriptorSet(descriptorSet);
		
		vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eCompute;
		if (m_pLastPipelineLayout->isGraphicsPipeline()) {
			bindPoint = vk::PipelineBindPoint::eGraphics;
		}
		const vk::PipelineLayout* pLayout = GetPipelineLayout(m_pLastPipelineLayout->getLayoutID());

		const vk::DescriptorSet set = pDescriptorSet->getSet(m_pCommandBufferSet->getBufferIndex());
		m_pCommandBufferSet->getBuffer().bindDescriptorSets(bindPoint, *pLayout, pDescriptorSet->getSetIndex(), 1U, &set, 0, nullptr);
	}

	void RenderContext::pushConstants(ShaderStageFlags stages, uint32_t offset, uint32_t size, const void* data) const {
		vk::PipelineLayout* pLayout = GetPipelineLayout(m_pLastPipelineLayout->getLayoutID());

		if (offset != UINT32_MAX) {
			m_pCommandBufferSet->getBuffer().pushConstants(*pLayout, static_cast<vk::ShaderStageFlags>(stages), offset, size, data);
		}
		else {
			const auto& pushConstantRanges = m_pLastPipelineLayout->getPushConstantRanges();
			for (auto range : pushConstantRanges) {
				if (range.stageFlags & stages) {
					if (range.offset < offset)
						offset = range.offset;
				}
			}
			m_pCommandBufferSet->getBuffer().pushConstants(*pLayout, static_cast<vk::ShaderStageFlags>(stages), offset, size, data);
		}
	}

	void RenderContext::setScissor(Rect scissor) const {
		vk::Rect2D rect { { scissor.offset.x, scissor.offset.y }, { scissor.extent.width, scissor.extent.height } };

		m_pCommandBufferSet->getBuffer().setScissor(0, 1, &rect);
	}

	void RenderContext::setViewport(Rect viewport) const {
		vk::Viewport vp = {};
		vp.width = viewport.extent.width;
		vp.height = viewport.extent.height;
		vp.maxDepth = 1.0f;
		vp.minDepth = 0.0f;
		vp.x = viewport.offset.x;
		vp.y = viewport.offset.y;
		m_pCommandBufferSet->getBuffer().setViewport(0, 1, &vp);
	}

	void RenderContext::setViewports(const Rect* pViewports, uint32_t viewportCount) const {
		std::array<vk::Viewport, 8> viewports;
		if (viewportCount > viewports.size())
			throw std::runtime_error("viewportCount was more than maximum number of viewports");
		for(uint32_t i = 0; i < viewportCount; i++) {
			vk::Viewport vp = {};
			vp.width = pViewports[i].extent.width;
			vp.height = pViewports[i].extent.height;
			vp.maxDepth = 1.0f;
			vp.minDepth = 0.0f;
			vp.x = pViewports[i].offset.x;
			vp.y = pViewports[i].offset.y;
			viewports[i] = vp;
		}
		m_pCommandBufferSet->getBuffer().setViewportWithCount(viewportCount, viewports.data());
	}

	void RenderContext::setDepthBias(float constantFactor, float clamp, float slopeFactor) const {
		m_pCommandBufferSet->getBuffer().setDepthBias(constantFactor, clamp, slopeFactor);
	}

	void RenderContext::setDepthBiasEnable(bool enable) const {
		m_pCommandBufferSet->getBuffer().setDepthBiasEnable(enable);
	}

	void RenderContext::setPrimitiveTopology(Topology topology) const {
		m_pCommandBufferSet->getBuffer().setPrimitiveTopology(static_cast<vk::PrimitiveTopology>(topology));
	}

	void RenderContext::setPatchControlPoints(uint32_t points) const {
		vkCmdSetPatchControlPointsEXT(m_pCommandBufferSet->getBuffer(), points);
	}

	void RenderContext::setPrimitiveRestartEnable(bool enable) const {
		m_pCommandBufferSet->getBuffer().setPrimitiveRestartEnable(enable);
	}

	void RenderContext::setRasterizerDiscardEnable(bool enable) const {
		m_pCommandBufferSet->getBuffer().setRasterizerDiscardEnable(enable);
	}

	void RenderContext::setCullMode(CullModeFlags cullMode) const {
		m_pCommandBufferSet->getBuffer().setCullMode(static_cast<vk::CullModeFlags>(cullMode));
	}

	void RenderContext::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const {
		m_pCommandBufferSet->getBuffer().draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void RenderContext::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) const {
		m_pCommandBufferSet->getBuffer().drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void RenderContext::drawIndexedIndirect(const Buffer& buffer, uint32_t offset, uint32_t drawCount, uint32_t stride) const {
		const DeviceBuffer* pBuffer = (const DeviceBuffer*)buffer;
		m_pCommandBufferSet->getBuffer().drawIndexedIndirect(pBuffer->buffer, offset, drawCount, stride);
	}

	void RenderContext::drawIndexedIndirect(const Buffer& buffer, size_t offset, const Buffer& countBuffer, size_t countOffset, uint32_t stride, uint32_t maxDrawCount) const {
		const DeviceBuffer* pBuffer = (const DeviceBuffer*)buffer;
		const DeviceBuffer* pCountBuffer = (const DeviceBuffer*)countBuffer;

		m_pCommandBufferSet->getBuffer().drawIndexedIndirectCount(pBuffer->buffer, offset, pCountBuffer->buffer, countOffset, maxDrawCount, stride);
	}

	void RenderContext::drawIndirect(const Buffer& buffer, uint32_t offset, uint32_t drawCount, uint32_t stride) const {
		const DeviceBuffer* pBuffer = (const DeviceBuffer*)buffer;
		m_pCommandBufferSet->getBuffer().drawIndirect(pBuffer->buffer, offset, drawCount, stride);
	}

	void RenderContext::drawIndirect(const Buffer& buffer, size_t offset, const Buffer& countBuffer, size_t countOffset, uint32_t stride, uint32_t maxDrawCount) const {
		const DeviceBuffer* pBuffer = (const DeviceBuffer*)buffer;
		const DeviceBuffer* pCountBuffer = (const DeviceBuffer*)countBuffer;
		m_pCommandBufferSet->getBuffer().drawIndirectCount(pBuffer->buffer, offset, pCountBuffer->buffer, countOffset, maxDrawCount, stride);
	}

	void RenderContext::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const {
		m_pCommandBufferSet->getBuffer().dispatch(groupCountX, groupCountY, groupCountZ);
	}

	void RenderContext::barrierDepthAttachmentToCompute(const Texture& texture) const {
		DeviceImage* pImage = texture;
		vk::ImageMemoryBarrier imageBarrier = {};
		imageBarrier.srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		imageBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		imageBarrier.oldLayout = pImage->layout;
		imageBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if (pImage->usage & vk::ImageUsageFlagBits::eStorage) {
			imageBarrier.newLayout = vk::ImageLayout::eGeneral;
		}
		
		imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eDepth;
		if (VulkanCore::HasStencilComponent(pImage->format)) {
			aspect |= vk::ImageAspectFlagBits::eStencil;
		}
		
		imageBarrier.image = pImage->image;
		imageBarrier.subresourceRange = {
				.aspectMask = aspect,
				.baseMipLevel = 0,
				.levelCount = pImage->mipLevels,
				.baseArrayLayer = 0,
				.layerCount = pImage->arrayLayers,
		};

		m_pCommandBufferSet->getBuffer().pipelineBarrier(
			vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
			vk::PipelineStageFlagBits::eComputeShader,
			(vk::DependencyFlags)0, 
			0, nullptr,
			0, nullptr, 
			1, &imageBarrier);

		pImage->layout = imageBarrier.newLayout;
	}

	void RenderContext::barrierComputeToFragment(const Texture& texture) const {
		DeviceImage* pImage = texture;
		vk::ImageMemoryBarrier imageBarrier = {};
		imageBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
		imageBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		imageBarrier.oldLayout = pImage->layout;
		imageBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if (pImage->usage & vk::ImageUsageFlagBits::eStorage) {
			imageBarrier.newLayout = vk::ImageLayout::eGeneral;
		}

		imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor;
		if (VulkanCore::IsDepthFormat(pImage->format)) {
			aspect = vk::ImageAspectFlagBits::eDepth;
			
			if (VulkanCore::HasStencilComponent(pImage->format)) {
				aspect |= vk::ImageAspectFlagBits::eStencil;
			}
		}

		imageBarrier.image = pImage->image;
		imageBarrier.subresourceRange = {
				.aspectMask = aspect,
				.baseMipLevel = 0,
				.levelCount = pImage->mipLevels,
				.baseArrayLayer = 0,
				.layerCount = pImage->arrayLayers,
		};

		m_pCommandBufferSet->getBuffer().pipelineBarrier(
			vk::PipelineStageFlagBits::eComputeShader,
			vk::PipelineStageFlagBits::eFragmentShader,
			(vk::DependencyFlags)0,
			0, nullptr,
			0, nullptr,
			1, &imageBarrier);

		pImage->layout = imageBarrier.newLayout;
	}

	void RenderContext::barrierColorCompute(const Texture& texture) const {

		DeviceImage* pImage = (DeviceImage*)texture;
		vk::ImageLayout newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if (texture.getUsageFlags() & sa::TextureUsageFlagBits::STORAGE)
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

	void RenderContext::barrierColorCompute(const Buffer& buffer) const {

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

	void RenderContext::barrier(const Texture& texture, Transition src, Transition dst) const {
		const ImageView& imageView = texture.getView();
		const DeviceImage* pImage = imageView.getImage();
		

		vk::AccessFlags srcAccess;
		vk::AccessFlags dstAccess;
		vk::PipelineStageFlags srcStage;
		vk::PipelineStageFlags dstStage;
		vk::ImageLayout oldLayout = vk::ImageLayout::eUndefined;
		vk::ImageLayout newLayout = vk::ImageLayout::eUndefined;

		VulkanCore::GetTransitionInfo(src, &srcStage, &srcAccess, &oldLayout);
		VulkanCore::GetTransitionInfo(dst, &dstStage, &dstAccess, &newLayout);

		vk::Format vkFormat = static_cast<vk::Format>(imageView.getFormat());

		vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor;
		if (VulkanCore::IsDepthFormat(vkFormat)) {
			aspectFlags = vk::ImageAspectFlagBits::eDepth;
			if (VulkanCore::HasStencilComponent(vkFormat)) {
				aspectFlags |= vk::ImageAspectFlagBits::eStencil;
			}
		}

		if (newLayout == vk::ImageLayout::eShaderReadOnlyOptimal 
			&& (texture.getUsageFlags() & sa::TextureUsageFlagBits::STORAGE) == sa::TextureUsageFlagBits::STORAGE) {
			newLayout = vk::ImageLayout::eGeneral;
		}
		
		vk::ImageMemoryBarrier imageBarrier{
			.srcAccessMask = srcAccess,
			.dstAccessMask = dstAccess,
			.oldLayout = oldLayout,
			.newLayout = newLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = pImage->image,
			.subresourceRange{
				.aspectMask = aspectFlags,
				.baseMipLevel = imageView.getBaseMipLevel(),
				.levelCount = imageView.getMipLevelCount(),
				.baseArrayLayer = imageView.getBaseArrayLayer(),
				.layerCount = imageView.getArrayLayerCount(),
			},
		};

		m_pCommandBufferSet->getBuffer().pipelineBarrier(
			srcStage,
			dstStage,
			(vk::DependencyFlags)0,
			nullptr,
			nullptr,
			imageBarrier);
#if 0
		std::cout << std::setfill('-') << std::setw(32) << "Image" << std::setw(32) << "srcAccess" << std::setw(32) << "dstAccess" << std::setw(16) << "mip" << std::setw(16) << "layer" << std::endl
			<< std::setfill(' ')
			<< std::setw(32) << pImage->image
			<< std::setw(32) << vk::to_string(srcAccess)
			<< std::setw(32) << vk::to_string(dstAccess)
			<< std::setw(16) << imageView.getBaseMipLevel()
			<< std::setw(16) << imageView.getBaseArrayLayer()
			<< std::endl
			<< "\t" << vk::to_string(oldLayout) << " -> " << vk::to_string(newLayout)
			<< std::endl;
#endif
	}

	void RenderContext::barrier(uint32_t textureCount, const Texture* pTextures, Transition src, Transition dst) const {
		std::vector<vk::ImageMemoryBarrier> memoryBarriers(textureCount);
		vk::AccessFlags srcAccess;
		vk::AccessFlags dstAccess;
		vk::PipelineStageFlags srcStage;
		vk::PipelineStageFlags dstStage;
		vk::ImageLayout oldLayout = vk::ImageLayout::eUndefined;
		vk::ImageLayout newLayout = vk::ImageLayout::eUndefined;

		VulkanCore::GetTransitionInfo(src, &srcStage, &srcAccess, &oldLayout);
		VulkanCore::GetTransitionInfo(dst, &dstStage, &dstAccess, &newLayout);

		for (uint32_t i = 0; i < textureCount; ++i) {

			const ImageView& imageView = pTextures[i].getView();
			const DeviceImage* pImage = imageView.getImage();

			vk::Format vkFormat = static_cast<vk::Format>(imageView.getFormat());

			vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor;
			if (VulkanCore::IsDepthFormat(vkFormat)) {
				aspectFlags = vk::ImageAspectFlagBits::eDepth;
				if (VulkanCore::HasStencilComponent(vkFormat)) {
					aspectFlags |= vk::ImageAspectFlagBits::eStencil;
				}
			}

			if (newLayout == vk::ImageLayout::eShaderReadOnlyOptimal
				&& (pTextures[i].getUsageFlags() & sa::TextureUsageFlagBits::STORAGE) == sa::TextureUsageFlagBits::STORAGE) {
				newLayout = vk::ImageLayout::eGeneral;
			}

			memoryBarriers[i] = {
				.srcAccessMask = srcAccess,
				.dstAccessMask = dstAccess,
				.oldLayout = oldLayout,
				.newLayout = newLayout,
				.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
				.image = pImage->image,
				.subresourceRange{
					.aspectMask = aspectFlags,
					.baseMipLevel = imageView.getBaseMipLevel(),
					.levelCount = imageView.getMipLevelCount(),
					.baseArrayLayer = imageView.getBaseArrayLayer(),
					.layerCount = imageView.getArrayLayerCount(),
				},
			};
		}

		m_pCommandBufferSet->getBuffer().pipelineBarrier(
			srcStage,
			dstStage,
			(vk::DependencyFlags)0,
			nullptr,
			nullptr,
			memoryBarriers);

		
	}

	void RenderContext::barrier(const Buffer& buffer, Transition src, Transition dst) const {
		const DeviceBuffer* pBuffer = buffer;

		vk::AccessFlags srcAccess;
		vk::AccessFlags dstAccess;
		vk::PipelineStageFlags srcStage;
		vk::PipelineStageFlags dstStage;
		vk::ImageLayout tmpLayout;

		VulkanCore::GetTransitionInfo(src, &srcStage, &srcAccess, &tmpLayout);
		VulkanCore::GetTransitionInfo(dst, &dstStage, &dstAccess, &tmpLayout);

		vk::BufferMemoryBarrier bufferBarrier{
			.srcAccessMask = srcAccess,
			.dstAccessMask = dstAccess,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.buffer = pBuffer->buffer,
			.offset = 0,
			.size = pBuffer->size,
		};
		
		m_pCommandBufferSet->getBuffer().pipelineBarrier(
			srcStage,
			dstStage,
			(vk::DependencyFlags)0,
			nullptr,
			bufferBarrier,
			nullptr);
	}

	void RenderContext::barrier(Transition src, Transition dst) const {
		vk::PipelineStageFlags srcStage;
		vk::PipelineStageFlags dstStage;
		vk::AccessFlags tmpAccess;
		vk::ImageLayout tmpLayout;

		VulkanCore::GetTransitionInfo(src, &srcStage, &tmpAccess, &tmpLayout);
		VulkanCore::GetTransitionInfo(dst, &dstStage, &tmpAccess, &tmpLayout);

		m_pCommandBufferSet->getBuffer().pipelineBarrier(
			srcStage,
			dstStage,
			(vk::DependencyFlags)0,
			nullptr,
			nullptr,
			nullptr);
	}

	void RenderContext::fullBarrier() const {
		vk::PipelineStageFlags srcStage = vk::PipelineStageFlagBits::eAllCommands;
		vk::PipelineStageFlags dstStage = vk::PipelineStageFlagBits::eAllCommands;

		vk::MemoryBarrier memorybarrier{
			.srcAccessMask = vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite,
			.dstAccessMask = vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite,
		};

		m_pCommandBufferSet->getBuffer().pipelineBarrier(
			srcStage,
			dstStage,
			(vk::DependencyFlags)0,
			memorybarrier,
			nullptr,
			nullptr);
	}

	void RenderContext::copyImageToImageColor(const Texture& src, const Texture& dst) const {
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
	
	void RenderContext::copyImageToSwapchain(const Texture& src, ResourceID swapchain) const {
		const DeviceImage* pSrc = src;
		Swapchain* pSwapchain = GetSwapchain(swapchain);
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

	void RenderContext::syncFramebuffer(ResourceID framebuffer) {
		FramebufferSet* pFramebufferSet = RenderContext::GetFramebufferSet(framebuffer);
		pFramebufferSet->sync(*this);
	}

	SubContext::SubContext()
		: RenderContext()
	{
	}

	SubContext::SubContext(VulkanCore* pCore, FramebufferSet* pFramebufferSet, RenderProgram* pRenderProgram, uint32_t subpassIndex, ResourceID contextPool) {
		m_pCore = pCore;

		if (contextPool == NULL_RESOURCE) {
			m_commandBufferSetID = ResourceManager::Get().insert(m_pCore->allocateCommandBufferSet(vk::CommandBufferLevel::eSecondary));
		}
		else {
			CommandPool* pool = ResourceManager::Get().get<CommandPool>(contextPool);
			m_commandBufferSetID = ResourceManager::Get().insert(m_pCore->allocateCommandBufferSet(vk::CommandBufferLevel::eSecondary, *pool));
		}
		m_pCommandBufferSet = ResourceManager::Get().get<CommandBufferSet>(m_commandBufferSetID);

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
		ResourceManager::Get().remove<CommandBufferSet>(m_commandBufferSetID);
		m_pCommandBufferSet = nullptr;
	}

	DirectContext::DirectContext(VulkanCore* pCore, ResourceID contextPool) {
		m_pCore = pCore;
		
		if (contextPool == NULL_RESOURCE) {
			m_commandBufferSetID = ResourceManager::Get().insert(m_pCore->allocateCommandBufferSet(vk::CommandBufferLevel::ePrimary));
		}
		else {
			CommandPool* pool = ResourceManager::Get().get<CommandPool>(contextPool);
			m_commandBufferSetID = ResourceManager::Get().insert(m_pCore->allocateCommandBufferSet(vk::CommandBufferLevel::ePrimary, *pool));
		}


		m_pCommandBufferSet = ResourceManager::Get().get<CommandBufferSet>(m_commandBufferSetID);

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
		ResourceManager::Get().remove<CommandBufferSet>(m_commandBufferSetID);
		m_pCommandBufferSet = nullptr;
	}


}
