#pragma once
#include "DescriptorSetStructs.h"
#include "ApiBuildOptions.h"
#include <map>

namespace spirv_cross {
	class Compiler;
}

namespace sa {
	class VulkanCore;

	// OBS: Will cause memory leak! This a an issue with shaderc_compiler
	[[nodiscard]] std::vector<uint32_t> CompileGLSLFromFile(const char* glslPath, ShaderStageFlagBits shaderStage, const char* entryPointName = "main", const char* tag = "Unamned Source");
	// OBS: Will cause memory leak! This a an issue with shaderc_compiler
	[[nodiscard]] std::vector<uint32_t> CompileGLSLFromMemory(const char* glslCode, ShaderStageFlagBits shaderStage, const char* entryPointName = "main", const char* tag = "Unamned Source");

	[[nodiscard]] std::vector<uint32_t> ReadSPVFile(const char* spvPath);

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

		void createShaderModule(const ShaderStageInfo& stageInfo);
		void initializeStage(spirv_cross::Compiler* pCompiler, ShaderStageFlagBits stage);
		void createDescriptorPoolAndLayouts();

	public:
		ShaderSet();
		ShaderSet(const ShaderSet&) = default;
		ShaderSet& operator=(const ShaderSet&) = default;

		ShaderSet(VulkanCore* pCore, const ShaderStageInfo* pStageInfos, uint32_t stageCount);
		ShaderSet(VulkanCore* pCore, const std::vector<ShaderStageInfo>& stageInfos);
		ShaderSet(VulkanCore* pCore, const std::vector<std::vector<uint32_t>>& shaderCode);

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