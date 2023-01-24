#include "pch.h"
#include "MainRenderLayer.h"

namespace sa {
	MainRenderLayer::MainRenderLayer() {

	}

	void MainRenderLayer::init(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique) {
		m_pWindow = pWindow;
		m_pRenderTechnique = pRenderTechnique;
	}

	void MainRenderLayer::cleanup() {

	}

	void MainRenderLayer::preRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget) {
		m_pRenderTechnique->prepareRender(context, pCamera, pRenderTarget);
	}

	void MainRenderLayer::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget) {
		Renderer& renderer = Renderer::get();
		
		m_pRenderTechnique->render(context, pCamera, pRenderTarget);

		pRenderTarget->outputTexture = &pRenderTarget->mainRenderData.colorTexture;

		m_pRenderTechnique->drawData.colorTexture = *pRenderTarget->outputTexture;
		m_pRenderTechnique->drawData.finalTexture = m_pRenderTechnique->drawData.colorTexture;

	}

	void MainRenderLayer::postRender(RenderContext& context) {
		m_pRenderTechnique->endRender(context);
	}

	void MainRenderLayer::onWindowResize(Extent newExtent) {

	}

	const Texture2D& MainRenderLayer::getOutputTexture() const {
		return {};
	}

}