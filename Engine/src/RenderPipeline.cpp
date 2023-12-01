#include "pch.h"
#include "Graphics/RenderPipeline.h"

namespace sa {

	RenderPipeline::RenderPipeline() {

	}

	RenderPipeline::~RenderPipeline() {
		for(auto& layer : m_renderLayers) {
			delete layer;
		}
	}

	void RenderPipeline::addLayer(IRenderLayer* pLayer) {
		m_renderLayers.push_back(pLayer);
		pLayer->init();
	}

	void RenderPipeline::onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent) {
		for (auto& layer : m_renderLayers) {
			layer->onRenderTargetResize(renderTargetID, oldExtent, newExtent);
		}
	}

	void RenderPipeline::beginFrameImGUI() {
		Renderer::get().newImGuiFrame();
	}

	void RenderPipeline::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) {
		SA_PROFILE_FUNCTION();
		if (!pRenderTarget->isActive())
			return;

		sceneCollection.makeRenderReady();


		for(auto& layer : m_renderLayers) {
			if (!layer->isActive())
				continue;
			if (!layer->preRender(context, pCamera, pRenderTarget, sceneCollection))
				return;
		}

		for (auto& layer : m_renderLayers) {
			if (!layer->isActive())
				continue;
			if (!layer->render(context, pCamera, pRenderTarget, sceneCollection))
				return;
		}

		for (auto& layer : m_renderLayers) {
			if(!layer->isActive())
				continue;
			if (!layer->postRender(context, pCamera, pRenderTarget, sceneCollection))
				return;
		}
		
		sceneCollection.swap();
		pRenderTarget->m_wasResized = false;
	}


}
