#include "pch.h"
#include "RenderContext.hpp"

#include "Resources/RenderProgram.hpp"
#include "Resources/FramebufferSet.hpp"
#include "Resources/Pipeline.hpp"
#include "Resources/Swapchain.hpp"


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
	{
	}

	RenderContext::RenderContext(VulkanCore* pCore, CommandBufferSet* pCommandBufferSet)
		: m_pCommandBufferSet(pCommandBufferSet)
		, m_pCore(pCore)
	{
	
	}

	void RenderContext::beginRenderProgram(ResourceID renderProgram, ResourceID framebuffer, Rect renderArea) {
		RenderProgram* pRenderProgram = getRenderProgram(renderProgram);
		FramebufferSet* pFramebuffer = getFramebufferSet(framebuffer);
		pRenderProgram->begin(m_pCommandBufferSet, pFramebuffer, renderArea);
	}

	void RenderContext::nextSubpass() {
		m_pCommandBufferSet->getBuffer().nextSubpass(vk::SubpassContents::eInline);
	}

	void RenderContext::endRenderProgram(ResourceID renderProgram) {
		RenderProgram* pRenderProgram = getRenderProgram(renderProgram);
		pRenderProgram->end(m_pCommandBufferSet);
	}

	void RenderContext::bindPipeline(ResourceID pipeline) {
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
				DEBUG_LOG_ERROR("All buffers must be of VERTEX type");
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
		pDescriptorSet->update(binding, pDeviceBuffer->buffer, pDeviceBuffer->size, 0, m_pCommandBufferSet->getBufferIndex());
	}

	void RenderContext::bindDescriptorSet(ResourceID descriptorSet, ResourceID pipeline) {
		Pipeline* pPipeline = getPipeline(pipeline);
		DescriptorSet* pDescriptorSet = getDescriptorSet(descriptorSet);
		pPipeline->bindDescriptorSet(m_pCommandBufferSet, pDescriptorSet);
	}

	void RenderContext::pushConstants(ResourceID pipeline, ShaderStageFlags stages, uint32_t offset, uint32_t size, void* data) {
		Pipeline* pPipeline = getPipeline(pipeline);
		pPipeline->pushConstants(m_pCommandBufferSet, (vk::ShaderStageFlags)stages, offset, size, data);
	}

	void RenderContext::bindDescriptorSets(const std::vector<ResourceID>& descriptorSets, ResourceID pipeline) {
		Pipeline* pPipeline = getPipeline(pipeline);
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

	void RenderContext::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
		m_pCommandBufferSet->getBuffer().draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void RenderContext::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) {
		m_pCommandBufferSet->getBuffer().drawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void RenderContext::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) {
		m_pCommandBufferSet->getBuffer().dispatch(groupCountX, groupCountY, groupCountZ);
	}

	void RenderContext::barrier(const Texture& texture) {

		DeviceImage* pImage = (DeviceImage*)texture;
		vk::ImageLayout newLayout = vk::ImageLayout::eGeneral;


		vk::ImageMemoryBarrier imageBarrier{
			.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
			.dstAccessMask = vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderWrite,
			.oldLayout = pImage->layout,
			.newLayout = newLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = pImage->image,
			.subresourceRange{
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.levelCount = 1,
				.layerCount = 1,
			},
		};

		pImage->layout = newLayout;

		m_pCommandBufferSet->getBuffer().pipelineBarrier(
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eComputeShader,
			(vk::DependencyFlags)0,
			nullptr,
			nullptr,
			imageBarrier);
	}

	void RenderContext::transitionTexture(const Texture& texture, Transition src, Transition dst) {

		vk::AccessFlags srcAccess;
		vk::AccessFlags dstAccess;
		vk::PipelineStageFlags srcStage;
		vk::PipelineStageFlags dstStage;
		vk::ImageLayout newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		if (texture.getTypeFlags() & TextureTypeFlagBits::STORAGE) {
			newLayout = vk::ImageLayout::eGeneral;
		}
		//TODO
		switch (src)
		{
		case sa::Transition::NONE:
			srcAccess = (vk::AccessFlags)0;
			break;
		case sa::Transition::RENDER_PROGRAM_INPUT:
			srcAccess = vk::AccessFlagBits::eInputAttachmentRead;
			break;
		case sa::Transition::RENDER_PROGRAM_OUTPUT:
			srcAccess = vk::AccessFlagBits::eColorAttachmentWrite;
			break;
		case sa::Transition::COMPUTE_SHADER_READ:
			srcAccess = vk::AccessFlagBits::eShaderRead;
			break;
		case sa::Transition::COMPUTE_SHADER_WRITE:
			srcAccess = vk::AccessFlagBits::eShaderWrite;
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
			vk::ImageAspectFlagBits::eColor,
			srcStage,
			dstStage
		);
	}

	Context::Context(VulkanCore* pCore, ResourceID commandBufferSetID) 
		: RenderContext(
			pCore, 
			ResourceManager::get().get<CommandBufferSet>(commandBufferSetID))
	{
		m_pFence = std::shared_ptr<vk::Fence>(new vk::Fence, [=](vk::Fence* p) {
			m_pCore->getDevice().destroyFence(*p);
			delete p;
		});
		*m_pFence = m_pCore->getDevice().createFence({ .flags = vk::FenceCreateFlagBits::eSignaled });
		m_commandBufferSetID = commandBufferSetID;
	}

	void Context::begin() {
		m_pCommandBufferSet->begin(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
	}

	void Context::end() {
		m_pCommandBufferSet->end();
	}

	void Context::submit() {
		waitToFinish();
		m_pCore->getDevice().resetFences(*m_pFence);
		m_pCommandBufferSet->submit(*m_pFence);
	}

	void Context::waitToFinish(size_t timeout) {
		m_pCore->getDevice().waitForFences(*m_pFence, VK_FALSE, timeout);
	}

	void Context::destroy() {
		ResourceManager::get().remove<CommandBufferSet>(m_commandBufferSetID);
		m_pCommandBufferSet = nullptr;
	}

}
