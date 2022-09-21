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

		m_pRenderTechnique->init(pWindow);
	}

	void RenderPipeline::pushLayer(IRenderLayer* pLayer) {
		m_layers.push_back(pLayer);
		pLayer->init(m_pWindow, m_pRenderTechnique);
	}

	void RenderPipeline::beginFrameImGUI() {
		Renderer::get().newImGuiFrame();
	}

	bool RenderPipeline::render(Scene* pScene) {
		
		//m_executor.wait_for_all();
		// collect meshes
		//m_taskflow.clear();
		
		m_pRenderTechnique->updateLights(pScene);
		m_pRenderTechnique->collectMeshes(pScene);
		m_cameras = pScene->getActiveCameras();

		//m_executor.async([&]() {
		m_context = m_pWindow->beginFrame();
		if (!m_context)
			return false;
		
		m_pRenderTechnique->updateData(m_context);

		for (auto cam : m_cameras) {
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
		//});

		return true;
	}

	IRenderTechnique* RenderPipeline::getRenderTechnique() const {
		return m_pRenderTechnique;
	}


	IRenderLayer::IRenderLayer()
		: m_renderer(Renderer::get())
	{
	}

	const Texture2D& IRenderLayer::getOutputTexture() const {
		return m_outputTexture;
	}

}