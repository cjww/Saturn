#include "pch.h"
#include "BloomRenderLayer.h"

namespace sa {
	void BloomRenderLayer::init(RenderWindow* pWindow, IRenderTechnique* pPreviousLayer) {

		/*
		assert(pPreviousLayer && "pPreviousLayer can not be nullptr");
		m_inputTexture = pPreviousLayer->getOutputTexture();

		m_blurComputePipeline = m_renderer.createComputePipeline("../Engine/shaders/GaussianBlur.comp.spv");
		
		m_descriptorSet = m_renderer.allocateDescriptorSet(m_blurComputePipeline, 0);

		m_outputTexture = m_renderer.createTexture2D(TextureTypeFlagBits::STORAGE, m_inputTexture.getExtent());
		m_renderer.updateDescriptorSet(m_descriptorSet, 0, m_inputTexture);
		m_renderer.updateDescriptorSet(m_descriptorSet, 1, m_outputTexture);
		*/

	}

	void BloomRenderLayer::cleanup() {

	}

	void BloomRenderLayer::onWindowResize(Extent newExtent) {
		
	}
}
