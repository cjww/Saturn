#include "pch.h"
#include "Resources/Pipeline.hpp"

namespace sa {
	Pipeline::Pipeline(VulkanCore* pCore, vk::RenderPass renderPass, const ShaderSet& shaderSet, uint32_t subpassIndex, Extent extent, PipelineConfig config) {
		create(pCore, renderPass, shaderSet, subpassIndex, extent, config);
	}

	void Pipeline::create(VulkanCore* pCore, vk::RenderPass renderPass, const ShaderSet& shaderSet, uint32_t subpassIndex, Extent extent, PipelineConfig config) {
		m_pCore = pCore;
		m_shaderSet = shaderSet;

		vk::PipelineLayoutCreateInfo layoutInfo;
		layoutInfo
			.setPushConstantRanges(shaderSet.getPushConstantRanges())
			.setSetLayouts(shaderSet.getDescriptorSetLayouts());
		m_layout = m_pCore->getDevice().createPipelineLayout(layoutInfo);

		vk::PipelineVertexInputStateCreateInfo vertexInput;
		vertexInput
			.setVertexAttributeDescriptions(shaderSet.getVertexAttributes())
			.setVertexBindingDescriptions(shaderSet.getVertexBindings());

		m_pipeline = m_pCore->createGraphicsPipeline(
			m_layout,
			renderPass,
			subpassIndex,
			{ extent.width, extent.height },
			shaderSet.getShaderInfos(),
			vertexInput,
			nullptr,
			config
		);

	}

	void Pipeline::destroy() {
		m_pCore->getDevice().destroyPipeline(m_pipeline);
		m_pCore->getDevice().destroyPipelineLayout(m_layout);
		m_shaderSet.destroy();
	}

	void Pipeline::bind(CommandBufferSet* cmd) {
		vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eGraphics;
		if (isCompute())
			bindPoint = vk::PipelineBindPoint::eCompute;
		cmd->getBuffer().bindPipeline(bindPoint, m_pipeline);
	}

	DescriptorSet Pipeline::allocateDescriptSet(uint32_t setIndex, uint32_t count) {
		return m_shaderSet.allocateDescriptorSet(setIndex, count);
	}

	bool Pipeline::isCompute() {
		return !m_shaderSet.isGraphicsSet();
	}
}