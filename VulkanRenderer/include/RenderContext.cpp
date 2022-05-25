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

	RenderContext::RenderContext()
		: m_pCommandBufferSet(nullptr)
	{
	}

	RenderContext::RenderContext(CommandBufferSet* pCommandBufferSet)
		: m_pCommandBufferSet(pCommandBufferSet)
	{

	}


	void RenderContext::beginRenderProgram(ResourceID renderProgram, ResourceID framebuffer, Rect renderArea) {
		RenderProgram* pRenderProgram = getRenderProgram(renderProgram);
		FramebufferSet* pFramebuffer = getFramebufferSet(framebuffer);
		pRenderProgram->begin(m_pCommandBufferSet, pFramebuffer, renderArea);
	}


	void RenderContext::endRenderProgram(ResourceID renderProgram) {
		RenderProgram* pRenderProgram = getRenderProgram(renderProgram);
		pRenderProgram->end(m_pCommandBufferSet);
	}

	void RenderContext::bindPipeline(ResourceID pipeline) {
		Pipeline* pPipeline = getPipeline(pipeline);
		pPipeline->bind(m_pCommandBufferSet);
	}

	void RenderContext::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
		m_pCommandBufferSet->getBuffer().draw(vertexCount, instanceCount, firstVertex, firstInstance);
	}
}
