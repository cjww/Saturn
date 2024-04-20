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

	void RenderPipeline::addLayer(BasicRenderLayer* pLayer) {
		m_renderLayers.push_back(pLayer);
		pLayer->init();
	}

	void RenderPipeline::onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent) {
		for (auto& layer : m_renderLayers) {
			layer->onRenderTargetResize(renderTargetID, oldExtent, newExtent);
		}
	}

	void RenderPipeline::updatePreferences() {
		for (auto& layer : m_renderLayers) {
			layer->onPreferencesUpdated();
		}
	}

	void RenderPipeline::beginFrameImGUI() {
		Renderer::Get().newImGuiFrame();
	}

	void RenderPipeline::preRender(RenderContext& context, SceneCollection& sceneCollection) {
		SA_PROFILE_FUNCTION();
		
		for (auto& layer : m_renderLayers) {
			if (!layer->isActive())
				continue;
			if (!layer->preRender(context, sceneCollection))
				return;
		}

	}

	void RenderPipeline::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) {
		SA_PROFILE_FUNCTION();
		if (!pRenderTarget->isActive())
			return;

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

		pRenderTarget->makeSampleReady(context);
		pRenderTarget->m_wasResized = false;
	}

}
