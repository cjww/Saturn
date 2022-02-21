#pragma once


#include "common.hpp"
#include "vulkan_base.hpp"

#include <memory>
#include <unordered_map>

namespace NAME_SPACE {

	struct DescriptorSetLayout {
		std::vector<VkDescriptorSetLayoutBinding> bindings;
		std::vector<VkWriteDescriptorSet> writes;
		std::vector<size_t> sizes;
	};

	class Shader {
	private:
		spirv_cross::Compiler* m_pCompiler;
		VkPipelineShaderStageCreateInfo m_info;
		VkDevice m_device;
		VkShaderStageFlagBits m_stage;

		std::vector<DescriptorSetLayout> m_descriptorSets;
		std::vector<VkPushConstantRange> m_pushConstantRanges;

		std::vector<VkVertexInputAttributeDescription> m_vertexAttributes;
		std::vector<VkVertexInputBindingDescription> m_vertexBindings;

		void addResources(const spirv_cross::SmallVector<spirv_cross::Resource>& resources, VkDescriptorType type, VkSampler* immutableSamplers);

		void getVertexInput(const spirv_cross::SmallVector<spirv_cross::Resource>& resources);

	public:
		Shader(const Shader&) = delete;
		Shader operator=(const Shader&) = delete;
		
		Shader(VkDevice device, const char* path, VkShaderStageFlagBits stage);
		virtual ~Shader();

		static std::vector<uint32_t> readCode(const char* path);

		void load(const char* path);

		VkShaderStageFlagBits getStage() const;
		const std::vector<DescriptorSetLayout>& getDescriptorSetLayouts() const;
		const std::vector<VkPushConstantRange>& getPushConstantRanges() const;

		VkPipelineShaderStageCreateInfo getInfo() const;

		const std::vector<VkVertexInputAttributeDescription>& getVertexAttributes() const;
		const std::vector<VkVertexInputBindingDescription>& getVertexBindings() const;

	};

	typedef std::shared_ptr<Shader> ShaderPtr;
}