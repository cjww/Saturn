#pragma once
#include "DescriptorSetStructs.h"
#include "ApiBuildOptions.h"
#include <map>
#include <set>

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

	[[nodiscard]] std::string ReadFile(const char* path);


	enum class ShaderAttributeType {
		UNKNOWN,
		VOID,
		BOOLEAN,
		SBYTE,
		UBYTE,
		SHORT,
		USHORT,
		INT,
		UINT,
		INT64,
		UINT64,
		ATOMIC_COUNTER,
		HALF,
		FLOAT,
		DOUBLE,
		STRUCT,
		IMAGE,
		SAMPLED_IMAGE,
		SAMPLER,
		ACCELERATION_STRUCTURE,
		RAY_QUERY
	};

	struct ShaderAttribute {
		DescriptorType descriptorType;
		ShaderAttributeType type;
		std::vector<uint32_t> arraySize;
		uint32_t vecSize;
		uint32_t columns;
		size_t size;
		uint32_t offset;
		std::string name;

		uint32_t set;
		uint32_t binding;
	};

	class ShaderSet {
	private:
		VulkanCore* m_pCore;

		std::vector<ShaderModuleInfo> m_shaderModules;

		std::map<uint32_t, ResourceID> m_descriptorSetLayouts;

		std::unordered_map<uint32_t, DescriptorSetLayoutInfo> m_descriptorSetLayoutInfos;
		std::vector<PushConstantRange> m_pushConstantRanges;
		
		std::vector<VertexInputAttributeDescription> m_vertexAttributes;
		std::vector<VertexInputBindingDescription> m_vertexBindings;

		std::unordered_map<std::string, ShaderAttribute> m_attributes;

		std::set<ResourceID> m_allocatedDescriptorSets;

		ResourceID m_descriptorPool;

		bool m_isGraphicsSet;
		bool m_hasTessellationStage;

		void createShaderModule(const ShaderStageInfo& stageInfo);
		void initializeStage(spirv_cross::Compiler* pCompiler, ShaderStageFlagBits stage);
		void createDescriptorPoolAndLayouts();

	public:
		ShaderSet();
		ShaderSet(const ShaderSet&) = delete;
		ShaderSet& operator=(const ShaderSet&) = delete;
		~ShaderSet();

		ShaderSet(const ShaderStageInfo* pStageInfos, uint32_t stageCount);
		ShaderSet(const std::vector<ShaderStageInfo>& stageInfos);
		ShaderSet(const std::vector<std::vector<uint32_t>>& shaderCode);

		bool isValid() const;

		void create(const ShaderStageInfo* pStageInfos, uint32_t stageCount);
		void create(const std::vector<ShaderStageInfo>& stageInfos);
		void create(const std::vector<std::vector<uint32_t>>& shaderCode);

		void destroy();

		const std::map<uint32_t, ResourceID>& getDescriptorSetLayouts() const;
		const std::vector<PushConstantRange>& getPushConstantRanges() const;

		const std::vector<ShaderModuleInfo>& getShaderModules() const;

		const std::vector<VertexInputAttributeDescription>& getVertexAttributes() const;
		const std::vector<VertexInputBindingDescription>& getVertexBindings() const;

		const ShaderAttribute& getShaderAttribute(const std::string& attributePath) const;

		const std::unordered_map<std::string, ShaderAttribute>& getShaderAttributes() const;


		bool isGraphicsSet() const;
		bool hasTessellationStage() const;

		ResourceID allocateDescriptorSet(uint32_t setIndex);

		bool hasAllocatedDescriptorSet(ResourceID descriptorSet);
	};

}