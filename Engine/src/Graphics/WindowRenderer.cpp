#include "pch.h"
#include "WindowRenderer.h"

#include "Engine.h"

namespace sa {

	WindowRenderer::WindowRenderer()
		: m_pWindow(nullptr)
		, m_swapchainFramebuffer(NULL_RESOURCE)
		, m_swapchainPipeline(NULL_RESOURCE)
		, m_swapchainRenderProgram(NULL_RESOURCE)
		, m_swapchainDescriptorSet(NULL_RESOURCE)
		, m_sampler(NULL_RESOURCE)
	{
	}

	void WindowRenderer::create(RenderWindow* pTargetWindow) {
		m_pWindow = pTargetWindow;
		onWindowResize(m_pWindow->getCurrentExtent());
		m_sampler = sa::Renderer::get().createSampler(FilterMode::LINEAR);
	}


	void WindowRenderer::onWindowResize(Extent extent) {
		assert(m_pWindow != nullptr && "Create was not called");

		auto& renderer = Renderer::get();
		if(m_swapchainDescriptorSet != NULL_RESOURCE)
			renderer.freeDescriptorSet(m_swapchainDescriptorSet);
		if (m_swapchainPipeline != NULL_RESOURCE)
			renderer.destroyPipeline(m_swapchainPipeline);
		if (m_swapchainFramebuffer != NULL_RESOURCE)
			renderer.destroyFramebuffer(m_swapchainFramebuffer);
		if (m_swapchainRenderProgram != NULL_RESOURCE)
			renderer.destroyRenderProgram(m_swapchainRenderProgram);


		m_swapchainRenderProgram = renderer.createRenderProgram()
			.addSwapchainAttachment(m_pWindow->getSwapchainID())
			.beginSubpass()
			.addAttachmentReference(0, SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.addColorDependency(SA_SUBPASS_EXTERNAL, 0)
			.end();

		m_swapchainPipeline = renderer.createGraphicsPipeline(m_swapchainRenderProgram, 0, extent,
			(Engine::getShaderDirectory() / "TransferToSwapchain.vert.spv").generic_string(), (Engine::getShaderDirectory() / "TransferToSwapchain.frag.spv").generic_string());

		std::vector<Texture> textures;
		m_swapchainFramebuffer = renderer.createSwapchainFramebuffer(m_swapchainRenderProgram, m_pWindow->getSwapchainID(), textures);
		m_swapchainDescriptorSet = renderer.allocateDescriptorSet(m_swapchainPipeline, 0);
	}

	void WindowRenderer::render(RenderContext& context, const Texture& texture) {
		SA_PROFILE_FUNCTION();
		
		// render texture to swapchain
		context.updateDescriptorSet(m_swapchainDescriptorSet, 0, texture, m_sampler);
		context.beginRenderProgram(m_swapchainRenderProgram, m_swapchainFramebuffer, SubpassContents::DIRECT);
		context.bindPipeline(m_swapchainPipeline);
		context.bindDescriptorSet(m_swapchainDescriptorSet, m_swapchainPipeline);
		context.draw(6, 1);
		context.endRenderProgram(m_swapchainRenderProgram);
		
	}
}