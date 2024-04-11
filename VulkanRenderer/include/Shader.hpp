#pragma once
#include "DescriptorSetStructs.h"
#include "ApiBuildOptions.h"
#include "PipelineLayout.hpp"

namespace sa {
	class VulkanCore;

	class Shader {
	private:
		VulkanCore* m_pCore;

		ResourceID m_shaderObject = NULL_RESOURCE;
		ResourceID m_shaderModule = NULL_RESOURCE;

		ShaderStageInfo m_stageInfo;

		void createModule();
		void createObject(ShaderStageFlags nextStages, const PipelineLayout& layout);

		void initShaderStageInfo(const std::vector<uint32_t>& shaderCode, ShaderStageFlagBits stage, const char* entryPointName);

	public:
		Shader();
		~Shader();

		Shader(const Shader&);
		Shader& operator=(const Shader& other);
		

		void create(const ShaderStageInfo& stageInfo);

		void create(const std::vector<uint32_t>& shaderCode, const char* entryPointName = "main");
		void create(const std::vector<uint32_t>& shaderCode, ShaderStageFlagBits stage, const char* entryPointName = "main");

		void create(const std::vector<uint32_t> &shaderCode, ShaderStageFlagBits stage, 
			ShaderStageFlags nextStages, const PipelineLayout& layout);

		bool isValid() const;

		void destroy();

		ResourceID getShaderModuleID() const;
		ResourceID getShaderObjectID() const;

		const ShaderStageInfo& getShaderStageInfo() const;
		ShaderStageFlagBits getStage() const;
	};

}