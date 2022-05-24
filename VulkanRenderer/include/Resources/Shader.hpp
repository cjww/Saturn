#pragma once

namespace sa {

	struct DescriptorSetLayout {
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		std::vector<vk::WriteDescriptorSet> writes;
		std::vector<size_t> sizes;
	};

	class Shader {
	private:
		spirv_cross::Compiler* m_pCompiler;
		vk::PipelineShaderStageCreateInfo m_info;
		vk::Device m_device;
		vk::ShaderStageFlagBits m_stage;

		std::vector<DescriptorSetLayout> m_descriptorSets;
		std::vector<vk::PushConstantRange> m_pushConstantRanges;

		std::vector<vk::VertexInputAttributeDescription> m_vertexAttributes;
		std::vector<vk::VertexInputBindingDescription> m_vertexBindings;

		void addResources(const spirv_cross::SmallVector<spirv_cross::Resource>& resources, vk::DescriptorType type, vk::Sampler* immutableSamplers);

		void getVertexInput(const spirv_cross::SmallVector<spirv_cross::Resource>& resources);

	public:
		Shader(const Shader&) = delete;
		Shader operator=(const Shader&) = delete;
		
		Shader(vk::Device device, const char* path, vk::ShaderStageFlagBits stage);
		virtual ~Shader();

		static std::vector<uint32_t> readCode(const char* path);

		void load(const char* path);

		vk::ShaderStageFlagBits getStage() const;
		const std::vector<DescriptorSetLayout>& getDescriptorSetLayouts() const;
		const std::vector<vk::PushConstantRange>& getPushConstantRanges() const;

		vk::PipelineShaderStageCreateInfo getInfo() const;

		const std::vector<vk::VertexInputAttributeDescription>& getVertexAttributes() const;
		const std::vector<vk::VertexInputBindingDescription>& getVertexBindings() const;

	};

	typedef std::shared_ptr<Shader> ShaderPtr;
}