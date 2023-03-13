#pragma once

#include "internal\DescriptorSet.hpp"

#define NOMINMAX
#include <spirv_cross/spirv_cross.hpp>


namespace sa {

	class ShaderModule {
	private:
		vk::PipelineShaderStageCreateInfo m_info;
		vk::Device m_device;
		vk::ShaderStageFlagBits m_stage;

		std::vector<DescriptorSetLayout> m_descriptorSets;
		std::vector<vk::PushConstantRange> m_pushConstantRanges;

		std::vector<vk::VertexInputAttributeDescription> m_vertexAttributes;
		std::vector<vk::VertexInputBindingDescription> m_vertexBindings;

		void addResources(const spirv_cross::Compiler& compiler, const spirv_cross::SmallVector<spirv_cross::Resource>& resources, vk::DescriptorType type, vk::Sampler* immutableSamplers);

		void getVertexInput(const spirv_cross::Compiler& compiler, const spirv_cross::SmallVector<spirv_cross::Resource>& resources);

	public:
		
		ShaderModule() = default;
		ShaderModule(const ShaderModule& shader) = default;
		ShaderModule& operator=(const ShaderModule&) = default;

		ShaderModule(vk::Device device, const char* path, vk::ShaderStageFlagBits stage);
		
		void create(vk::Device device, const char* path, vk::ShaderStageFlagBits stage);
		void destroy();

		static std::vector<uint32_t> readCode(const char* path);

		void load(const char* path);

		vk::ShaderStageFlagBits getStage() const;
		const std::vector<DescriptorSetLayout>& getDescriptorSetLayouts() const;
		const std::vector<vk::PushConstantRange>& getPushConstantRanges() const;

		vk::PipelineShaderStageCreateInfo getInfo() const;

		const std::vector<vk::VertexInputAttributeDescription>& getVertexAttributes() const;
		const std::vector<vk::VertexInputBindingDescription>& getVertexBindings() const;

	};

}