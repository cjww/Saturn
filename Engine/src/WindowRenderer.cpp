#include "pch.h"
#include "Graphics/WindowRenderer.h"

#include "Engine.h"

namespace sa {
	
	IWindowRenderer::~IWindowRenderer() {
	
	}

	WindowRenderer::WindowRenderer(RenderWindow* pWindow)
		: m_pWindow(pWindow)
		, m_swapchainFramebuffer(NULL_RESOURCE)
		, m_swapchainPipeline(NULL_RESOURCE)
		, m_swapchainRenderProgram(NULL_RESOURCE)
		, m_swapchainDescriptorSet(NULL_RESOURCE)
		, m_sampler(NULL_RESOURCE)
	{
		auto& renderer = sa::Renderer::Get();
		auto vertexCode = ReadSPVFile((Engine::GetShaderDirectory() / "TransferToSwapchain.vert.spv").generic_string().c_str());
		auto fragmentCode = ReadSPVFile((Engine::GetShaderDirectory() / "TransferToSwapchain.frag.spv").generic_string().c_str());

		m_vertexShader.create(vertexCode, VERTEX);
		m_fragmentShader.create(fragmentCode, FRAGMENT);

		m_swapchainPipelineLayout.createFromShaders({ m_vertexShader, m_fragmentShader });

		m_sampler = renderer.createSampler(FilterMode::LINEAR);

		m_swapchainRenderProgram = renderer.createRenderProgram()
			.addSwapchainAttachment(m_pWindow->getSwapchainID())
			.beginSubpass()
			.addAttachmentReference(0, SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.addColorDependency(SA_SUBPASS_EXTERNAL, 0)
			.end();


		std::array<sa::Shader, 2> shaders = { m_vertexShader, m_fragmentShader };
		sa::PipelineSettings settings = {};
		settings.dynamicStates.push_back(sa::DynamicState::VIEWPORT);
		m_swapchainPipeline = renderer.createGraphicsPipeline(
			m_swapchainPipelineLayout, 
			shaders.data(), 
			shaders.size(), 
			m_swapchainRenderProgram, 
			0, 
			m_pWindow->getCurrentExtent());
		m_swapchainDescriptorSet = m_swapchainPipelineLayout.allocateDescriptorSet(0);
		
		onWindowResize(m_pWindow->getCurrentExtent());
	}

	void WindowRenderer::onWindowResize(Extent extent) {
		auto& renderer = Renderer::Get();
		if (m_swapchainFramebuffer != NULL_RESOURCE)
			renderer.destroyFramebuffer(m_swapchainFramebuffer);

		
		m_swapchainFramebuffer = renderer.createSwapchainFramebuffer(m_swapchainRenderProgram, m_pWindow->getSwapchainID());

		SA_DEBUG_LOG_INFO("Resized window renderer");
	}

	void WindowRenderer::render(RenderContext& context, const Texture& texture) {
		SA_PROFILE_FUNCTION();
		assert(texture.isValid() && "Texture must be valid");

		// render texture to swapchain
		context.updateDescriptorSet(m_swapchainDescriptorSet, 0, texture, m_sampler);
		context.beginRenderProgram(m_swapchainRenderProgram, m_swapchainFramebuffer, SubpassContents::DIRECT);
		context.bindPipelineLayout(m_swapchainPipelineLayout);
		context.bindPipeline(m_swapchainPipeline);
		Rect viewport = {};
		viewport.offset = { 0, 0 };
		viewport.extent = m_pWindow->getCurrentExtent();
		context.setViewport(viewport);
		context.bindDescriptorSet(m_swapchainDescriptorSet);
		context.draw(6, 1);
		context.endRenderProgram(m_swapchainRenderProgram);
	}
}