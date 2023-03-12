#include "pch.h"
#include "RenderPipeline.h"

namespace sa {

	RenderPipeline::RenderPipeline() {
		m_pBloomPass = std::make_unique<BloomRenderLayer>();
		m_pBloomPass->init();
	}

	RenderPipeline::~RenderPipeline() {
		delete m_pRenderTechnique;
	}

	void RenderPipeline::create(IRenderTechnique* pRenderTechnique) {
		m_pRenderTechnique = pRenderTechnique;
		m_pRenderTechnique->init();
	}

	void RenderPipeline::beginFrameImGUI() {
		Renderer::get().newImGuiFrame();
	}

	void RenderPipeline::render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) {
		SA_PROFILE_FUNCTION();
		if (!pRenderTarget->isActive())
			return;

		sceneCollection.makeRenderReady();

		
		m_pRenderTechnique->preRender(context, pCamera, pRenderTarget, sceneCollection);
		m_pRenderTechnique->render(context, pCamera, pRenderTarget, sceneCollection);

		if(m_pBloomPass->isActive())
			m_pBloomPass->render(context, pCamera, pRenderTarget, sceneCollection);

		sceneCollection.swap();
		pRenderTarget->swap();
	}

	IRenderTechnique* RenderPipeline::getRenderTechnique() const {
		return m_pRenderTechnique;
	}

	BloomRenderLayer* RenderPipeline::getBloomPass() const {
		return m_pBloomPass.get();
	}


}