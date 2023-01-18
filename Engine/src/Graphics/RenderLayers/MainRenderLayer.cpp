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

	void MainRenderLayer::preRender(RenderContext& context, SceneCamera* pCamera) {
		m_pRenderTechnique->prepareRender(context, pCamera);
	}

	void MainRenderLayer::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget) {
		m_pRenderTechnique->drawData.colorTexture = Renderer::get().getFramebufferTexture(pRenderTarget->framebuffer, 0, context.getFrameIndex()); // We know the first texture has to be the color texture
		m_pRenderTechnique->drawData.finalTexture = m_pRenderTechnique->drawData.colorTexture;
		pRenderTarget->outputTexture = m_pRenderTechnique->drawData.colorTexture;
		m_pRenderTechnique->render(context, pCamera, pRenderTarget->framebuffer);
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