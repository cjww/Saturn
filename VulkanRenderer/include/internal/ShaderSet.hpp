#pragma once
#include "DescriptorSetStructs.h"
#include <map>

namespace sa {
	class VulkanCore;

	std::vector<uint32_t> readSpvFile(const char* spvPath);

	class ShaderSet {
	private:
		VulkanCore* m_pCore;

		std::vector<ShaderModuleInfo> m_shaderModules;

		std::map<uint32_t, ResourceID> m_descriptorSetLayouts;

		std::unordered_map<uint32_t, DescriptorSetLayoutInfo> m_descriptorSetLayoutInfos;
		std::vector<PushConstantRange> m_pushConstantRanges;
		
		std::vector<VertexInputAttributeDescription> m_vertexAttributes;
		std::vector<VertexInputBindingDescription> m_vertexBindings;

		ResourceID m_descriptorPool;

		bool m_isGraphicsSet;
		bool m_hasTessellationStage;

	public:
		ShaderSet() = default;
		ShaderSet(const ShaderSet&) = default;
		ShaderSet& operator=(const ShaderSet&) = default;

		ShaderSet(VulkanCore* pCore, const ShaderStageInfo* pStageInfos, uint32_t stageCount);

		void destroy();

		const std::map<uint32_t, ResourceID>& getDescriptorSetLayouts() const;
		const std::vector<PushConstantRange>& getPushConstantRanges() const;

		const std::vector<ShaderModuleInfo>& getShaderModules() const;

		const std::vector<VertexInputAttributeDescription>& getVertexAttributes() const;
		const std::vector<VertexInputBindingDescription>& getVertexBindings() const;

		bool isGraphicsSet() const;
		bool hasTessellationStage() const;

		ResourceID allocateDescriptorSet(uint32_t setIndex);
	};

}