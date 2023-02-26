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
	}

	void RenderPipeline::create(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique) {
		m_pRenderTechnique = pRenderTechnique;
		m_pWindow = pWindow;

		m_pRenderTechnique->init();
	}

	void RenderPipeline::pushLayer(IRenderLayer* pLayer) {
		m_layers.push_back(pLayer);
		pLayer->init(m_pRenderTechnique);
	}

	void RenderPipeline::pushOverlay(IRenderLayer* pLayer) {
		m_overlays.push_back(pLayer);
		pLayer->init(m_pRenderTechnique);
	}

	void RenderPipeline::beginFrameImGUI() {
		Renderer::get().newImGuiFrame();
	}

	void RenderPipeline::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) {
		SA_PROFILE_FUNCTION();

		for (auto& layer : m_layers) {
			if(layer->isActive())
				layer->preRender(context, pCamera, pRenderTarget, sceneCollection);
		}

		for (auto& layer : m_layers) {
			if(layer->isActive())
				layer->render(context, pCamera, pRenderTarget, sceneCollection);
		}

		pRenderTarget->swap();
	}

	const Texture& RenderPipeline::endScene(RenderContext& context) {
		SA_PROFILE_FUNCTION();

		for (auto& layer : m_layers) {
			if (layer->isActive())
				layer->postRender(context);
		}
		for (auto& layer : m_overlays) {
			if (layer->isActive())
				layer->postRender(context);
		}
		return m_pRenderTechnique->drawData.finalTexture;
	}

	IRenderTechnique* RenderPipeline::getRenderTechnique() const {
		return m_pRenderTechnique;
	}
}