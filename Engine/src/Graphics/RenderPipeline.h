#pragma once

#include "IRenderTechnique.h"
#include "IRenderLayer.h"
#include "Tools/Profiler.h"
#include "RenderLayers\BloomRenderLayer.h"

namespace sa {

	class RenderPipeline {
	private:
		IRenderTechnique* m_pRenderTechnique;
		
		std::unique_ptr<BloomRenderLayer> m_pBloomPass;

	public:
		RenderPipeline();
		virtual ~RenderPipeline();

		void create(IRenderTechnique* pRenderTechnique);
		
		void beginFrameImGUI();
		
		void render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection);

		IRenderTechnique* getRenderTechnique() const;

		BloomRenderLayer* getBloomPass() const;

	};

}
