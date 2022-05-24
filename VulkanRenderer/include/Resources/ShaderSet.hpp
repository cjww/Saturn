#pragma once
#include "Resources/Shader.hpp"

namespace sa {

	struct DescriptorSet {
		std::vector<vk::DescriptorSet> descriptorSets;
		std::vector<vk::WriteDescriptorSet> writes;
		uint32_t setIndex;
	};

	typedef std::shared_ptr<DescriptorSet> DescriptorSetPtr;

	class ShaderSet {
	private:
		vk::Device m_device;
		uint32_t m_swapChainImageCount;

		std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts;
		std::vector<vk::PushConstantRange> m_pushConstantRanges;
		std::vector<vk::PipelineShaderStageCreateInfo> m_shaderInfos;

		std::vector<DescriptorSetLayout> m_descriptorSets;

		vk::DescriptorPool m_descriptorPool;

		std::vector<vk::VertexInputAttributeDescription> m_vertexAttributes;
		std::vector<vk::VertexInputBindingDescription> m_vertexBindings;

		bool m_isGraphicsSet;

		ShaderPtr m_pVertexShader, m_pGeometryShader, m_pFragmentShader, m_pComputeShader;

	public:
		ShaderSet(const ShaderSet&) = delete;
		ShaderSet operator=(const ShaderSet&) = delete;

		ShaderSet(vk::Device device, uint32_t swapChainImageCount, const ShaderPtr& vertexShader, const ShaderPtr& fragmentShader);
		ShaderSet(vk::Device device, uint32_t swapChainImageCount, const ShaderPtr& vertexShader, const ShaderPtr& geometryShader, const ShaderPtr& fragmentShader);
		
		ShaderSet(vk::Device device, uint32_t swapChainImageCount, const ShaderPtr& computeShader);

		virtual ~ShaderSet();

		const std::vector<vk::DescriptorSetLayout>& getDescriptorSetLayouts() const;
		const std::vector<vk::PushConstantRange>& getPushConstantRanges() const;

		const std::vector<vk::PipelineShaderStageCreateInfo>& getShaderInfos() const;

		const std::vector<vk::VertexInputAttributeDescription>& getVertexAttributes() const;
		const std::vector<vk::VertexInputBindingDescription>& getVertexBindings() const;

		bool isGraphicsSet() const;

		DescriptorSetPtr getDescriptorSet(uint32_t setIndex);
		void destroyDescriptorSet(DescriptorSetPtr descriptorSet);

	};

	typedef std::shared_ptr<ShaderSet> ShaderSetPtr;
}