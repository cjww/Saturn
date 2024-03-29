#pragma once
#include <map>
#include <set>

#include "DescriptorSetStructs.h"
#include "PipelineSettings.hpp"
#include "ShaderInfoStructs.h"

namespace spirv_cross
{
	class Compiler;
}

namespace sa {

	// OBS: Will cause memory leak! This a an issue with shaderc_compiler
	[[nodiscard]] std::vector<uint32_t> CompileGLSLFromFile(const char* glslPath, ShaderStageFlagBits shaderStage, const char* entryPointName = "main", const char* tag = "Unamned Source");
	// OBS: Will cause memory leak! This a an issue with shaderc_compiler
	[[nodiscard]] std::vector<uint32_t> CompileGLSLFromMemory(const char* glslCode, ShaderStageFlagBits shaderStage, const char* entryPointName = "main", const char* tag = "Unamned Source");

	[[nodiscard]] std::vector<uint32_t> ReadSPVFile(const char* spvPath);

	[[nodiscard]] std::string ReadFile(const char* path);



	class VulkanCore;
	class PipelineLayout;
	class Shader;

	class DescriptorSetFactory {
		friend class PipelineLayout;
		uint32_t m_setIndex;
		DescriptorSetLayoutInfo m_set;
		PipelineLayout* m_playout;
		explicit DescriptorSetFactory(PipelineLayout* pLayout, uint32_t setIndex);
	public:
		DescriptorSetFactory() = delete;

		DescriptorSetFactory& addBinding(uint32_t binding, DescriptorType type, uint32_t count, ShaderStageFlags stageFlags, size_t size);

		void endDescriptorSet() const;
	};

	class PipelineLayout {
		VulkanCore* m_pCore;

		friend class DescriptorSetFactory;
		std::unordered_map<uint32_t, DescriptorSetLayoutInfo> m_descriptorSetLayoutInfos;
		std::map<uint32_t, ResourceID> m_descriptorSetLayouts;
		std::vector<PushConstantRange> m_pushConstantRanges;

		ResourceID m_layout;

		std::vector<VertexInputAttributeDescription> m_vertexAttributes;
		std::vector<VertexInputBindingDescription> m_vertexBindings;

		std::unordered_map<std::string, ShaderAttribute> m_attributes;

		std::set<ResourceID> m_allocatedDescriptorSets;

		ResourceID m_descriptorPool;

		bool m_isGraphicsPipeline;
		bool m_hasTessellationStage;

		void initializeStage(const ShaderStageInfo& stageInfo);
		void initializeStage(spirv_cross::Compiler* pCompiler, ShaderStageFlagBits stage);
		void createDescriptorPoolAndLayouts();
		void createPipelineLayout();
	public:
		PipelineLayout();
		PipelineLayout(const PipelineLayout&) = delete;
		PipelineLayout& operator=(const PipelineLayout&) = delete;
		~PipelineLayout();
		
		DescriptorSetFactory beginDescriptorSet(uint32_t setIndex);
		void create();
		
		void createFromShaders(const ShaderStageInfo* pStageInfos, uint32_t stageCount);
		void createFromShaders(const std::vector<ShaderStageInfo>& stageInfos);
		void createFromShaders(const std::vector<std::vector<uint32_t>>& shaderCode);
		void createFromShaders(const std::vector<uint32_t>& shaderCode);
		void createFromShaders(const std::vector<Shader>& shaders);
		void createFromShaders(const Shader* pShaders, uint32_t shaderCount);
		
		void destroy();

		bool isValid() const;

		const std::map<uint32_t, ResourceID>& getDescriptorSetLayouts() const;
		const std::vector<PushConstantRange>& getPushConstantRanges() const;

		const std::vector<VertexInputAttributeDescription>& getVertexAttributes() const;
		const std::vector<VertexInputBindingDescription>& getVertexBindings() const;

		const ShaderAttribute& getShaderAttribute(const std::string& attributePath) const;
		const std::unordered_map<std::string, ShaderAttribute>& getShaderAttributes() const;
		

		bool isGraphicsPipeline() const;
		bool hasTessellationStage() const;

		ResourceID allocateDescriptorSet(uint32_t setIndex);

		bool hasAllocatedDescriptorSet(ResourceID descriptorSet);
		
		ResourceID getLayoutID() const;

	};

}
