#include "pch.h"
#include "RenderPipeline.h"

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
	}

	void RenderPipeline::create(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique) {
		m_pRenderTechnique = pRenderTechnique;
		m_pWindow = pWindow;

		m_pRenderTechnique->init(pWindow->getCurrentExtent());

		Renderer& renderer = Renderer::get();

		m_swapchainRenderProgram = renderer.createRenderProgram()
			.addSwapchainAttachment(pWindow->getSwapchainID())
			.beginSubpass()
			.addAttachmentReference(0, SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.end();

		m_swapchainPipeline = renderer.createGraphicsPipeline(m_swapchainRenderProgram, 0, pWindow->getCurrentExtent(),
			"../Engine/shaders/TransferToSwapchain.vert.spv", "../Engine/shaders/TransferToSwapchain.frag.spv");

		m_swapchainRenderTarget.framebuffer = renderer.createSwapchainFramebuffer(m_swapchainRenderProgram, pWindow->getSwapchainID(), {});
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

	bool RenderPipeline::render(Scene* pScene) {
		SA_PROFILE_FUNCTION();
		/*
		m_pRenderTechnique->updateLights(pScene);
		// collect meshes
		m_pRenderTechnique->collectMeshes(pScene);
		auto cameras = pScene->getActiveCameras();

		m_context = m_pWindow->beginFrame();
		if (!m_context)
			return false;
		
		m_pRenderTechnique->updateData(m_context);

		for (auto cam : cameras) {
			m_pRenderTechnique->preRender(m_context, cam);
			for (auto& layer : m_layers) {
				layer->preRender(m_context, cam);
			}
			m_pRenderTechnique->render(m_context, cam);
			for (auto& layer : m_layers) {
				layer->render(m_context, cam);
			}
		}

		m_pRenderTechnique->postRender(m_context);
		for (auto& layer : m_layers) {
			layer->postRender(m_context);
		}

		m_pWindow->display();
		*/
		
		if (beginScene(pScene)) {

			auto cameras = pScene->getActiveCameras();
			for (auto& camera : cameras) {
				render(camera);
			}

			endScene();
			return true;
		}

		return false;
	}

	bool RenderPipeline::beginScene(Scene* pScene) {
		SA_PROFILE_FUNCTION();

		m_pRenderTechnique->updateLights(pScene);
		// collect meshes
		m_pRenderTechnique->collectMeshes(pScene);

		m_context = m_pWindow->beginFrame();
		if (!m_context)
			return false;
	
		m_pRenderTechnique->updateData(m_context);
	

		return true;
	}

	void RenderPipeline::render(Camera* pCamera) {
		SA_PROFILE_FUNCTION();
		// Depth prepass and light culling
		// generate shadowmaps
		for (auto& layer : m_layers) {
			layer->preRender(m_context, pCamera);
		}

		// render to all rendertargets
		// use m_pRenderTechnique->compose(context, pCamera, myFramebuffer) to render final image to own framebuffer because all data will be the same (i.e. lights and meshes)
		for (auto& layer : m_layers) {
			layer->render(m_context, pCamera);
		}

		// render main color image
		//m_pRenderTechnique->render(m_context, pCamera);

	}

	void RenderPipeline::endScene() {
		SA_PROFILE_FUNCTION();
		for (auto& layer : m_layers) {
			layer->postRender(m_context);
		}

		for (auto& layer : m_overlays) {
			layer->postRender(m_context);
		}

		m_context.updateDescriptorSet(m_swapchainDescriptorSet, 0, m_pRenderTechnique->drawData.finalTexture, m_sampler);

		m_context.beginRenderProgram(m_swapchainRenderProgram, m_swapchainRenderTarget.framebuffer, SubpassContents::DIRECT);
		m_context.bindPipeline(m_swapchainPipeline);
		m_context.bindDescriptorSet(m_swapchainDescriptorSet, m_swapchainPipeline);
		m_context.draw(6, 1);
		m_context.endRenderProgram(m_swapchainRenderProgram);

		m_pWindow->display();
	}

	IRenderTechnique* RenderPipeline::getRenderTechnique() const {
		return m_pRenderTechnique;
	}


	IRenderLayer::IRenderLayer()
		: m_renderer(Renderer::get())
	{
	}

}