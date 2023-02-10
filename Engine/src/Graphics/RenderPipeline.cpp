#include "pch.h"
#include "RenderPipeline.h"
#include "Engine.h"

namespace sa {

	RenderPipeline::RenderPipeline() {

	}

	RenderPipeline::~RenderPipeline() {
		delete m_pRenderTechnique;
		for (auto& layer : m_layers) {
			delete layer;
		}
		for (auto& layer : m_overlays) {
			delete layer;
		}
	}

	void RenderPipeline::onWindowResize(Extent newExtent) {
		m_pRenderTechnique->onWindowResize(newExtent);
		for (auto& layer : m_layers) {
			layer->onWindowResize(newExtent);
		}

		for (auto& layer : m_overlays) {
			layer->onWindowResize(newExtent);
		}

		auto& renderer = Renderer::get();
		renderer.freeDescriptorSet(m_swapchainDescriptorSet);
		renderer.destroyPipeline(m_swapchainPipeline);
		renderer.destroyFramebuffer(m_swapchainFramebuffer);
		renderer.destroyRenderProgram(m_swapchainRenderProgram);


		m_swapchainRenderProgram = renderer.createRenderProgram()
			.addSwapchainAttachment(m_pWindow->getSwapchainID())
			.beginSubpass()
			.addAttachmentReference(0, SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.addColorDependency(SA_SUBPASS_EXTERNAL, 0)
			.end();

		m_swapchainPipeline = renderer.createGraphicsPipeline(m_swapchainRenderProgram, 0, newExtent,
			(Engine::getShaderDirectory()  / "TransferToSwapchain.vert.spv").generic_string(), (Engine::getShaderDirectory() / "TransferToSwapchain.frag.spv").generic_string());

		std::vector<Texture> textures;
		m_swapchainFramebuffer = renderer.createSwapchainFramebuffer(m_swapchainRenderProgram, m_pWindow->getSwapchainID(), textures);
		m_swapchainDescriptorSet = renderer.allocateDescriptorSet(m_swapchainPipeline, 0);

	}

	void RenderPipeline::create(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique) {
		m_pRenderTechnique = pRenderTechnique;
		m_pWindow = pWindow;

		m_pRenderTechnique->init();

		Renderer& renderer = Renderer::get();

		m_swapchainRenderProgram = renderer.createRenderProgram()
			.addSwapchainAttachment(m_pWindow->getSwapchainID())
			.beginSubpass()
			.addAttachmentReference(0, SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.addColorDependency(SA_SUBPASS_EXTERNAL, 0)
			.end();

		m_swapchainPipeline = renderer.createGraphicsPipeline(m_swapchainRenderProgram, 0, m_pWindow->getCurrentExtent(),
			(Engine::getShaderDirectory() / "TransferToSwapchain.vert.spv").generic_string(), (Engine::getShaderDirectory() / "TransferToSwapchain.frag.spv").generic_string());
		std::vector<Texture> textures;
		m_swapchainFramebuffer = renderer.createSwapchainFramebuffer(m_swapchainRenderProgram, m_pWindow->getSwapchainID(), textures);
		m_swapchainDescriptorSet = renderer.allocateDescriptorSet(m_swapchainPipeline, 0);

		m_sampler = renderer.createSampler(FilterMode::LINEAR);

	}

	void RenderPipeline::pushLayer(IRenderLayer* pLayer) {
		m_layers.push_back(pLayer);
		pLayer->init(m_pWindow, m_pRenderTechnique);
	}

	void RenderPipeline::pushOverlay(IRenderLayer* pLayer) {
		m_overlays.push_back(pLayer);
		pLayer->init(m_pWindow, m_pRenderTechnique);
	}

	void RenderPipeline::beginFrameImGUI() {
		Renderer::get().newImGuiFrame();
	}

	RenderContext RenderPipeline::beginScene(Scene* pScene) {
		{
			SA_PROFILE_SCOPE("Begin frame i.e. wait for fence");
			m_context = m_pWindow->beginFrame();
			if (!m_context)
				return {};
		}
		if (pScene) {
			m_pRenderTechnique->updateLights(pScene);
			// collect meshes
			m_pRenderTechnique->collectMeshes(pScene);
		}
	
		m_pRenderTechnique->updateData(m_context);
		
		return m_context;
	}

	void RenderPipeline::render(SceneCamera* pCamera, RenderTarget* pRendertarget) {
		SA_PROFILE_FUNCTION();

		for (auto& layer : m_layers) {
			if(layer->isActive())
				layer->preRender(m_context, pCamera, pRendertarget);
		}

		for (auto& layer : m_layers) {
			if(layer->isActive())
				layer->render(m_context, pCamera, pRendertarget);
		}

		pRendertarget->swap();
	}

	void RenderPipeline::endScene() {
		SA_PROFILE_FUNCTION();

		for (auto& layer : m_layers) {
			if (layer->isActive())
				layer->postRender(m_context);
		}
		for (auto& layer : m_overlays) {
			if (layer->isActive())
				layer->postRender(m_context);
		}

		{
			SA_PROFILE_SCOPE("Render To Swapchain");
			// render finalTextuer to swapchain
			//m_context.copyImageToSwapchain(m_pRenderTechnique->drawData.finalTexture, m_pWindow->getSwapchainID());
			m_context.updateDescriptorSet(m_swapchainDescriptorSet, 0, m_pRenderTechnique->drawData.finalTexture, m_sampler);
			m_context.beginRenderProgram(m_swapchainRenderProgram, m_swapchainFramebuffer, SubpassContents::DIRECT);
			m_context.bindPipeline(m_swapchainPipeline);
			m_context.bindDescriptorSet(m_swapchainDescriptorSet, m_swapchainPipeline);
			m_context.draw(6, 1);
			m_context.endRenderProgram(m_swapchainRenderProgram);
		}
		
		{
			SA_PROFILE_SCOPE("Display");
			m_pWindow->display();
		}
	}

	IRenderTechnique* RenderPipeline::getRenderTechnique() const {
		return m_pRenderTechnique;
	}
}