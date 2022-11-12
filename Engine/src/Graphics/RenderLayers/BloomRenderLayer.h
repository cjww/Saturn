#pragma once
#include "Graphics/IRenderLayer.h"

namespace sa {
	class BloomRenderLayer : public IRenderLayer {
	private:
		ResourceID m_blurComputePipeline = NULL_RESOURCE;
		ResourceID m_descriptorSet = NULL_RESOURCE;
		Texture2D m_inputTexture;
		
	public:
		virtual void init(RenderWindow* pWindow, IRenderTechnique* pRenderTechnique = nullptr) override;
	
		virtual void cleanup() override;
		virtual void onWindowResize(Extent newExtent) override;

		virtual const Texture2D& getOutputTexture() const override;

	};
}

