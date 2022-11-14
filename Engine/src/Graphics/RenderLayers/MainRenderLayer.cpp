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

	void MainRenderLayer::preRender(RenderContext& context, Camera* pCamera) {
		m_pRenderTechnique->prepareRender(context, pCamera);
	}

	void MainRenderLayer::render(RenderContext& context, Camera* pCamera, RenderTarget* pRenderTarget) {
		m_pRenderTechnique->render(context, pCamera, pRenderTarget->framebuffer);
	}

	void MainRenderLayer::postRender(RenderContext& context) {
		m_pRenderTechnique->endRender(context);
	}

	void MainRenderLayer::onWindowResize(Extent newExtent) {

	}

	const Texture2D& MainRenderLayer::getOutputTexture() const {
		return m_pRenderTechnique->drawData.colorTexture;
	}

}