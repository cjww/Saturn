#include "pch.h"
#include "MainRenderLayer.h"

namespace sa {
	MainRenderLayer::MainRenderLayer() {

	}

	MainRenderLayer::MainRenderLayer(Texture2D texture)
		: m_texture(texture)
	{

	}

	void MainRenderLayer::init(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique) {
		m_pWindow = pWindow;
		m_pRenderTechnique = pRenderTechnique;
		if(!m_texture.isValid())
			m_texture = m_pRenderTechnique->drawData.colorTexture;

		m_sceneFramebuffer = m_pRenderTechnique->createColorFramebuffer(m_texture);

	}

	void MainRenderLayer::cleanup() {

	}

	void MainRenderLayer::preRender(RenderContext& context, Camera* pCamera) {
		m_pRenderTechnique->prepareRender(context, pCamera);
	}

	void MainRenderLayer::render(RenderContext& context, Camera* pCamera) {
		m_pRenderTechnique->render(context, pCamera, m_sceneFramebuffer);
		m_pRenderTechnique->drawData.colorTexture = m_texture;
	}

	void MainRenderLayer::postRender(RenderContext& context) {

	}

	void MainRenderLayer::onWindowResize(Extent newExtent) {

	}

	const Texture2D& MainRenderLayer::getOutputTexture() const {
		return m_pRenderTechnique->drawData.colorTexture;
	}

}