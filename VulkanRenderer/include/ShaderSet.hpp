#pragma once
#include "Shader.hpp"

#include <set>

namespace NAME_SPACE {

	struct DescriptorSet {
		std::vector<VkDescriptorSet> descriptorSets;
		std::vector<VkWriteDescriptorSet> writes;
		uint32_t setIndex;
	};

	typedef std::shared_ptr<DescriptorSet> DescriptorSetPtr;

	class ShaderSet {
	private:
		VkDevice m_device;
		uint32_t m_swapChainImageCount;

		std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
		std::vector<VkPushConstantRange> m_pushConstantRanges;
		std::vector<VkPipelineShaderStageCreateInfo> m_shaderInfos;

		std::vector<DescriptorSetLayout> m_descriptorSets;

		VkDescriptorPool m_descriptorPool;

		std::vector<VkVertexInputAttributeDescription> m_vertexAttributes;
		std::vector<VkVertexInputBindingDescription> m_vertexBindings;


	public:
		ShaderSet(const ShaderSet&) = delete;
		ShaderSet operator=(const ShaderSet&) = delete;

		ShaderSet(VkDevice device, uint32_t swapChainImageCount, const ShaderPtr& vertexShader, const ShaderPtr& fragmentShader);
		ShaderSet(VkDevice device, uint32_t swapChainImageCount, const ShaderPtr& vertexShader, const ShaderPtr& geometryShader, const ShaderPtr& fragmentShader);
		
		ShaderSet(VkDevice device, uint32_t swapChainImageCount, const ShaderPtr& compuetShader);

		virtual ~ShaderSet();

		const std::vector<VkDescriptorSetLayout>& getDescriptorSetLayouts() const;
		const std::vector<VkPushConstantRange>& getPushConstantRanges() const;

		const std::vector<VkPipelineShaderStageCreateInfo>& getShaderInfos() const;

		const std::vector<VkVertexInputAttributeDescription>& getVertexAttributes() const;
		const std::vector<VkVertexInputBindingDescription>& getVertexBindings() const;

		bool isGraphicsSet() const;

		DescriptorSetPtr getDescriptorSet(uint32_t setIndex);

	};

	typedef std::shared_ptr<ShaderSet> ShaderSetPtr;
}