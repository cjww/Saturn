#pragma once
#include "internal/ShaderModule.hpp"
#include "internal/DescriptorSet.hpp"


namespace sa {

	
	class ShaderSet {
	private:
		vk::Device m_device;
		
		std::vector<vk::DescriptorSetLayout> m_descriptorSetLayouts;
		std::vector<vk::PushConstantRange> m_pushConstantRanges;
		std::vector<vk::PipelineShaderStageCreateInfo> m_shaderInfos;

		std::vector<DescriptorSetLayout> m_descriptorSets;

		vk::DescriptorPool m_descriptorPool;

		std::vector<vk::VertexInputAttributeDescription> m_vertexAttributes;
		std::vector<vk::VertexInputBindingDescription> m_vertexBindings;

		bool m_isGraphicsSet;

		std::optional<ShaderModule> m_vertexShader, m_geometryShader, m_fragmentShader, m_computeShader;

		void init(const std::vector<ShaderModule>& shaders);

	public:
		ShaderSet() = default;
		ShaderSet(const ShaderSet&) = default;
		ShaderSet& operator=(const ShaderSet&) = default;

		ShaderSet(vk::Device device, const ShaderModule& vertexShader, const ShaderModule& fragmentShader);
		ShaderSet(vk::Device device, const ShaderModule& vertexShader, const ShaderModule& geometryShader, const ShaderModule& fragmentShader);
		ShaderSet(vk::Device device, const ShaderModule& computeShader);

		void destroy();

		const std::vector<vk::DescriptorSetLayout>& getDescriptorSetLayouts() const;
		const std::vector<vk::PushConstantRange>& getPushConstantRanges() const;

		const std::vector<vk::PipelineShaderStageCreateInfo>& getShaderInfos() const;

		const std::vector<vk::VertexInputAttributeDescription>& getVertexAttributes() const;
		const std::vector<vk::VertexInputBindingDescription>& getVertexBindings() const;

		bool isGraphicsSet() const;

		DescriptorSet allocateDescriptorSet(uint32_t setIndex, uint32_t count);
	};

}