#include "pch.h"
#include "BloomRenderLayer.h"

namespace sa {
	void BloomRenderLayer::init(RenderWindow* pWindow, IRenderLayer* pPreviousLayer) {
		assert(pPreviousLayer && "pPreviousLayer can not be nullptr");
		m_inputTexture = pPreviousLayer->getOutputTexture();

		m_blurComputePipeline = m_renderer.createComputePipeline("../Engine/shaders/GaussianBlur.comp.spv");
		
		/*
		m_descriptorSet = m_renderer.allocateDescriptorSet(m_blurComputePipeline, 0);

		m_renderer.updateDescriptorSet(m_descriptorSet, 0, m_inputTexture);
		m_renderer.updateDescriptorSet(m_descriptorSet, 1, m_blurredBrightnessTexture);
		*/
	}
}
