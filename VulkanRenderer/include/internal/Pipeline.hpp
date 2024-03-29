#pragma once

#include "ShaderSet.hpp"
#include "internal/DescriptorSet.hpp"
#include "VulkanCore.hpp"

namespace sa {
	/*
	class Pipeline {
	private:
		VulkanCore* m_pCore;

		vk::Pipeline m_pipeline;

	public:
		Pipeline();

		void create(vk::RenderPass renderPass, const ShaderSet& shaderSet, uint32_t subpassIndex, Extent extent, PipelineConfig config);
		void create(const ShaderSet& shaderSet, PipelineConfig config);

		void destroy();
	
		void bind(CommandBufferSet* cmd);

		void bindDescriptorSets(CommandBufferSet* cmd, std::vector<DescriptorSet*> sets);
		void bindDescriptorSets(CommandBufferSet* cmd, uint32_t firstSet, std::vector<vk::DescriptorSet> sets);

		void bindDescriptorSet(CommandBufferSet* cmd, DescriptorSet* set);

		void pushConstants(CommandBufferSet* cmd, vk::ShaderStageFlags stages, uint32_t offset, uint32_t size, void* data);
		void pushConstants(CommandBufferSet* cmd, vk::ShaderStageFlags stages, uint32_t size, void* data);

		bool isCompute();
	};
	*/


}