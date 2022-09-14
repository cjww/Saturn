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
		
		
		m_context = m_pWindow->beginFrame();
		if (!m_context)
			return false;
		
		m_pRenderTechnique->updateData(m_context, pScene);

		for (auto cam : pScene->getActiveCameras()) {
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
		return true;
	}

	IRenderTechnique* RenderPipeline::getRenderTechnique() const {
		return m_pRenderTechnique;
	}

	void ImGuiRenderLayer::init(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique) {
		m_pWindow = pWindow;
		Renderer& renderer = Renderer::get();
		m_imGuiRenderProgram = renderer.createRenderProgram()
			.addSwapchainAttachment(pWindow->getSwapchainID())
			.beginSubpass()
			.addAttachmentReference(0, sa::SubpassAttachmentUsage::ColorTarget)
			.endSubpass()
			.end();

		m_imGuiFramebuffer = renderer.createSwapchainFramebuffer(m_imGuiRenderProgram, pWindow->getSwapchainID(), {});
		renderer.initImGui(*pWindow, m_imGuiRenderProgram, 0);
	}

	void ImGuiRenderLayer::postRender(RenderContext& context) {
		context.beginRenderProgram(m_imGuiRenderProgram, m_imGuiFramebuffer, sa::SubpassContents::DIRECT);
		context.renderImGuiFrame();
		context.endRenderProgram(m_imGuiRenderProgram);
	}

	void ImGuiRenderLayer::onWindowResize(Extent newExtent) {
		Renderer::get().cleanupImGui();
		init(m_pWindow, nullptr);
	}

}