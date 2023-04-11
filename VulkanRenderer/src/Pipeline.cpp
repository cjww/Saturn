#include "pch.h"
#include "internal/Pipeline.hpp"

namespace sa {
	Pipeline::Pipeline(VulkanCore* pCore, vk::RenderPass renderPass, const ShaderSet& shaderSet, uint32_t subpassIndex, Extent extent, PipelineConfig config) {
		create(pCore, renderPass, shaderSet, subpassIndex, extent, config);
	}

	Pipeline::Pipeline(VulkanCore* pCore, const ShaderSet& shaderSet, PipelineConfig config) {
		create(pCore, shaderSet, config);
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

		if (shaderSet.hasTessellationStage())
			config.tessellation.enabled = true;

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

	void Pipeline::create(VulkanCore* pCore, const ShaderSet& shaderSet, PipelineConfig config) {
		m_pCore = pCore;
		m_shaderSet = shaderSet;

		vk::PipelineLayoutCreateInfo layoutInfo;
		layoutInfo
			.setPushConstantRanges(shaderSet.getPushConstantRanges())
			.setSetLayouts(shaderSet.getDescriptorSetLayouts());


		m_layout = m_pCore->getDevice().createPipelineLayout(layoutInfo);

		vk::ComputePipelineCreateInfo info{
			.stage = shaderSet.getShaderInfos()[0],
			.layout = m_layout,
		};
	
		m_pipeline = m_pCore->getDevice().createComputePipeline(nullptr, info).value;
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

	DescriptorSet Pipeline::allocateDescriptSet(uint32_t setIndex) {
		return m_shaderSet.allocateDescriptorSet(setIndex, m_pCore->getQueueCount());
	}

	void Pipeline::bindDescriptorSets(CommandBufferSet* cmd, std::vector<DescriptorSet*> sets) {
		if (sets.empty())
			return;
		vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eGraphics;
		if (isCompute()) {
			bindPoint = vk::PipelineBindPoint::eCompute;
		}

		std::vector<vk::DescriptorSet> descriptorSets;
		descriptorSets.reserve(sets.size());
		for (DescriptorSet* pSet : sets) {
			descriptorSets.push_back(pSet->getSet(cmd->getBufferIndex()));
		}

		cmd->getBuffer().bindDescriptorSets(bindPoint, m_layout, sets[0]->getSetIndex(), descriptorSets, nullptr);
	}

	void Pipeline::bindDescriptorSets(CommandBufferSet* cmd, uint32_t firstSet, std::vector<vk::DescriptorSet> sets) {
		if (sets.empty())
			return;
		vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eGraphics;
		if (isCompute()) {
			bindPoint = vk::PipelineBindPoint::eCompute;
		}
		cmd->getBuffer().bindDescriptorSets(bindPoint, m_layout, firstSet, sets, nullptr);
	}

	void Pipeline::bindDescriptorSet(CommandBufferSet* cmd, DescriptorSet* set) {
		vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eGraphics;
		if (isCompute()) {
			bindPoint = vk::PipelineBindPoint::eCompute;
		}
		cmd->getBuffer().bindDescriptorSets(bindPoint, m_layout, set->getSetIndex(), set->getSet(cmd->getBufferIndex()), nullptr);
	}

	void Pipeline::pushConstants(CommandBufferSet* cmd, vk::ShaderStageFlags stages, uint32_t offset, uint32_t size, void* data) {
		cmd->getBuffer().pushConstants(m_layout, stages, offset, size, data);
	}

	void Pipeline::pushConstants(CommandBufferSet* cmd, vk::ShaderStageFlags stages, uint32_t size, void* data) {
		uint32_t offset = UINT32_MAX;
		const auto& pushConstantRanges = m_shaderSet.getPushConstantRanges();
		for (auto range : pushConstantRanges) {
			if (range.stageFlags & stages) {
				if (range.offset < offset)
					offset = range.offset;
			}
		}
		cmd->getBuffer().pushConstants(m_layout, stages, offset, size, data);
	}

	bool Pipeline::isCompute() {
		return !m_shaderSet.isGraphicsSet();
	}
}