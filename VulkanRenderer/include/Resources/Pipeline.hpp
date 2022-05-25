#pragma once

#include "Resources/ShaderSet.hpp"
#include "VulkanCore.hpp"

namespace sa {

	class Pipeline {
	private:
		VulkanCore* m_pCore;

		vk::Pipeline m_pipeline;
		vk::PipelineLayout m_layout;
		ShaderSet m_shaderSet;

	public:
		Pipeline(VulkanCore* pCore, vk::RenderPass renderPass, const ShaderSet& shaderSet, uint32_t subpassIndex, Extent extent, PipelineConfig config);
		void create(VulkanCore* pCore, vk::RenderPass renderPass, const ShaderSet& shaderSet, uint32_t subpassIndex, Extent extent, PipelineConfig config);
		void destroy();
	
		void bind(CommandBufferSet* cmd);

		DescriptorSet allocateDescriptSet(uint32_t setIndex, uint32_t count);

		bool isCompute();
	};
}