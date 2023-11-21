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
		m_pShaderSet = (ShaderSet*)&shaderSet;

		vk::PipelineLayoutCreateInfo layoutInfo = {};

		auto& pushCostantRanges = shaderSet.getPushConstantRanges();

		std::vector<vk::PushConstantRange> vk_pushConstantRanges(pushCostantRanges.size());
		for (int i = 0; i < vk_pushConstantRanges.size(); i++) {
			vk_pushConstantRanges[i].offset = pushCostantRanges[i].offset;
			vk_pushConstantRanges[i].stageFlags = (vk::ShaderStageFlags)pushCostantRanges[i].stageFlags;
			vk_pushConstantRanges[i].size = pushCostantRanges[i].size;
		}

		auto& descriptorSetLayouts = shaderSet.getDescriptorSetLayouts();
		std::vector<vk::DescriptorSetLayout> vk_descriptorSetLayouts;

		auto back = descriptorSetLayouts.end();
		back--;
		uint32_t maxSet = back->first;

		for (uint32_t i = 0; i <= maxSet; i++) {
			if (!descriptorSetLayouts.count(i)) {
				SA_DEBUG_LOG_ERROR("Set index ", i, " does not exist! All sets from 0 to ", maxSet, " must be used!");
				throw std::runtime_error("Set index does not exist");
			}
			vk::DescriptorSetLayout* pLayout = ResourceManager::get().get<vk::DescriptorSetLayout>(descriptorSetLayouts.at(i));
			if (!pLayout) {
				SA_DEBUG_LOG_ERROR("Invalid descriptor layout ID ", descriptorSetLayouts.at(i));
				continue;
			}
			vk_descriptorSetLayouts.push_back(*pLayout);
		}

		layoutInfo.setPushConstantRanges(vk_pushConstantRanges);
		layoutInfo.setSetLayouts(vk_descriptorSetLayouts);
		m_layout = m_pCore->getDevice().createPipelineLayout(layoutInfo);

		auto& vertexAttributes = shaderSet.getVertexAttributes();

		std::vector<vk::VertexInputAttributeDescription> vk_vertexAttributes(vertexAttributes.size());
		for (int i = 0; i < vk_vertexAttributes.size(); i++) {
			vk_vertexAttributes[i].binding = vertexAttributes[i].binding;
			vk_vertexAttributes[i].format = (vk::Format)vertexAttributes[i].format;
			vk_vertexAttributes[i].location = vertexAttributes[i].location;
			vk_vertexAttributes[i].offset = vertexAttributes[i].offset;
		}


		auto& vertexBindings = shaderSet.getVertexBindings();
		std::vector<vk::VertexInputBindingDescription> vk_vertexBindings(vertexBindings.size());
		for (int i = 0; i < vk_vertexBindings.size(); i++) {
			vk_vertexBindings[i].binding = vertexBindings[i].binding;
			vk_vertexBindings[i].stride = vertexBindings[i].stride;
			vk_vertexBindings[i].inputRate = vk::VertexInputRate::eVertex;
		}

		vk::PipelineVertexInputStateCreateInfo vertexInput;
		vertexInput
			.setVertexAttributeDescriptions(vk_vertexAttributes)
			.setVertexBindingDescriptions(vk_vertexBindings);

		if (shaderSet.hasTessellationStage())
			config.tessellation.enabled = true;


		auto& shaderStageInfos = shaderSet.getShaderModules();
		std::vector<vk::PipelineShaderStageCreateInfo> vk_shaderStageInfos(shaderStageInfos.size());
		for (int i = 0; i < vk_shaderStageInfos.size(); i++) {
			vk::ShaderModule* pModule = sa::ResourceManager::get().get<vk::ShaderModule>(shaderStageInfos[i].moduleID);
			if (!pModule) {
				SA_DEBUG_LOG_ERROR("Invalid shader module ID ", shaderStageInfos[i].moduleID);
				continue;
			}
			vk_shaderStageInfos[i].module = *pModule;
			vk_shaderStageInfos[i].pName = shaderStageInfos[i].entryPointName.c_str();
			vk_shaderStageInfos[i].stage = (vk::ShaderStageFlagBits)shaderStageInfos[i].stage;
			vk_shaderStageInfos[i].pSpecializationInfo = nullptr;	
		}

		m_pipeline = m_pCore->createGraphicsPipeline(
			m_layout,
			renderPass,
			subpassIndex,
			{ extent.width, extent.height },
			vk_shaderStageInfos,
			vertexInput,
			nullptr,
			config
		);

	}

	void Pipeline::create(VulkanCore* pCore, const ShaderSet& shaderSet, PipelineConfig config) {
		m_pCore = pCore;
		m_pShaderSet = (ShaderSet*)&shaderSet;

		vk::PipelineLayoutCreateInfo layoutInfo = {};

		auto& pushCostantRanges = shaderSet.getPushConstantRanges();

		std::vector<vk::PushConstantRange> vk_pushConstantRanges(pushCostantRanges.size());
		for (int i = 0; i < vk_pushConstantRanges.size(); i++) {
			vk_pushConstantRanges[i].offset = pushCostantRanges[i].offset;
			vk_pushConstantRanges[i].stageFlags = (vk::ShaderStageFlags)pushCostantRanges[i].stageFlags;
			vk_pushConstantRanges[i].size = pushCostantRanges[i].size;
		}

		auto& descriptorSetLayouts = shaderSet.getDescriptorSetLayouts();
		std::vector<vk::DescriptorSetLayout> vk_descriptorSetLayouts;
		for (auto& [set, layoutID] : descriptorSetLayouts) {
			vk::DescriptorSetLayout* pLayout = ResourceManager::get().get<vk::DescriptorSetLayout>(layoutID);
			if (!pLayout) {
				SA_DEBUG_LOG_ERROR("Invalid descriptor layout ID ", layoutID);
				continue;
			}
			vk_descriptorSetLayouts.push_back(*pLayout);
		}


		layoutInfo.setPushConstantRanges(vk_pushConstantRanges);
		layoutInfo.setSetLayouts(vk_descriptorSetLayouts);
		m_layout = m_pCore->getDevice().createPipelineLayout(layoutInfo);

		auto& shaderStageInfos = shaderSet.getShaderModules();
		vk::PipelineShaderStageCreateInfo vk_shaderStageInfo;
	
		vk::ShaderModule* pModule = sa::ResourceManager::get().get<vk::ShaderModule>(shaderStageInfos[0].moduleID);
		if (!pModule) {
			SA_DEBUG_LOG_ERROR("Invalid shader module ID ", shaderStageInfos[0].moduleID);
		}
		vk_shaderStageInfo.module = *pModule;
		vk_shaderStageInfo.pName = shaderStageInfos[0].entryPointName.c_str();
		vk_shaderStageInfo.stage = (vk::ShaderStageFlagBits)shaderStageInfos[0].stage;
		vk_shaderStageInfo.pSpecializationInfo = nullptr;
	
		vk::ComputePipelineCreateInfo info{
			.stage = vk_shaderStageInfo,
			.layout = m_layout,
		};
	
		m_pipeline = m_pCore->getDevice().createComputePipeline(nullptr, info).value;
	}

	void Pipeline::destroy() {
		m_pCore->getDevice().destroyPipeline(m_pipeline);
		m_pCore->getDevice().destroyPipelineLayout(m_layout);
	}

	void Pipeline::bind(CommandBufferSet* cmd) {
		vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eGraphics;
		if (isCompute())
			bindPoint = vk::PipelineBindPoint::eCompute;
		cmd->getBuffer().bindPipeline(bindPoint, m_pipeline);
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
		const auto& pushConstantRanges = m_pShaderSet->getPushConstantRanges();
		for (auto range : pushConstantRanges) {
			if ((vk::ShaderStageFlags)range.stageFlags & stages) {
				if (range.offset < offset)
					offset = range.offset;
			}
		}
		cmd->getBuffer().pushConstants(m_layout, stages, offset, size, data);
	}

	bool Pipeline::isCompute() {
		return !m_pShaderSet->isGraphicsSet();
	}
}