#include "Shader.hpp"

namespace NAME_SPACE {
	void Shader::addResources(const spirv_cross::SmallVector<spirv_cross::Resource>& resources, VkDescriptorType type, VkSampler* immutableSamplers) {
		for (auto& b : resources) {
			VkDescriptorSetLayoutBinding layoutBinding = {};
			uint32_t set = m_pCompiler->get_decoration(b.id, spv::Decoration::DecorationDescriptorSet);
			layoutBinding.binding = m_pCompiler->get_decoration(b.id, spv::Decoration::DecorationBinding);
			size_t size = 0;
			if (type != VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE && 
				type != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER &&
				type != VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
				size = m_pCompiler->get_declared_struct_size(m_pCompiler->get_type(b.type_id));
			}
			layoutBinding.stageFlags = m_stage;

			auto t = m_pCompiler->get_type(b.type_id);

			layoutBinding.descriptorCount = (t.array.size() > 0) ? t.array[0] : 1;
			layoutBinding.descriptorType = type;
			layoutBinding.pImmutableSamplers = immutableSamplers;

			VkWriteDescriptorSet write = {};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.descriptorCount = layoutBinding.descriptorCount;
			write.descriptorType = layoutBinding.descriptorType;
			write.dstBinding = layoutBinding.binding;

			// Needs to be set when writing
			write.pImageInfo = nullptr;
			write.pTexelBufferView = nullptr;
			write.pBufferInfo = nullptr;
			write.dstArrayElement = 0;
			write.dstSet = VK_NULL_HANDLE;

			if (m_descriptorSets.size() <= set) {
				int diff = set - (m_descriptorSets.size() - 1);
				m_descriptorSets.resize(m_descriptorSets.size() + diff);
			}
			m_descriptorSets[set].bindings.push_back(layoutBinding);
			m_descriptorSets[set].writes.push_back(write);
			m_descriptorSets[set].sizes.push_back(size);
		}
	}

	void Shader::getVertexInput(const spirv_cross::SmallVector<spirv_cross::Resource>& resources) {

		uint32_t size = 0;
		std::unordered_map<uint32_t, uint32_t> sizes;
		std::unordered_map<uint32_t, VkVertexInputAttributeDescription> attribs;

		for (auto& input : resources) {
			auto type = m_pCompiler->get_type(input.type_id);
			uint32_t location = m_pCompiler->get_decoration(input.id, spv::Decoration::DecorationLocation);
			uint32_t binding = m_pCompiler->get_decoration(input.id, spv::Decoration::DecorationBinding);
			VkFormat format = VK_FORMAT_UNDEFINED;
			if (type.basetype == spirv_cross::SPIRType::BaseType::UInt) {
				format = VK_FORMAT_R32_UINT;
				sizes[location] = sizeof(unsigned int);
			}
			else if (type.basetype == spirv_cross::SPIRType::BaseType::Int || type.basetype == spirv_cross::SPIRType::BaseType::Boolean) {
				format = VK_FORMAT_R32_SINT;
				sizes[location] = sizeof(int);
			}
			else {
				int i = type.vecsize;
				switch (i) {
				case 1:
					format = VK_FORMAT_R32_SFLOAT;
					break;
				case 2:
					format = VK_FORMAT_R32G32_SFLOAT;
					break;
				case 3:
					format = VK_FORMAT_R32G32B32_SFLOAT;
					break;
				case 4:
					format = VK_FORMAT_R32G32B32A32_SFLOAT;
					break;
				}
				sizes[location] = sizeof(float) * i;

			}

			if (format == VK_FORMAT_UNDEFINED) {
				throw std::runtime_error("Undefined vertex input format");
			}

			VkVertexInputAttributeDescription attrib = {};
			attrib.binding = binding;
			attrib.location = location;
			attrib.format = format;
			attribs[location] = attrib;

			size += sizes[location];
		}

		for (auto& input : resources) {
			uint32_t location = m_pCompiler->get_decoration(input.id, spv::Decoration::DecorationLocation);
			uint32_t offset = 0;
			for (uint32_t i = 0; i < location; i++) {
				offset += sizes[i];
			}
			attribs[location].offset = offset;
		}

		for (auto& it : attribs) {
			m_vertexAttributes.push_back(it.second);
		}
		if (m_vertexAttributes.size() > 0) {
			VkVertexInputBindingDescription b = {};
			b.binding = 0;
			b.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			b.stride = size;

			m_vertexBindings = { b };
		}
	}

	Shader::Shader(VkDevice device, const char* path, VkShaderStageFlagBits stage) : m_device(device), m_stage(stage) {
		auto code = readCode(path);

        m_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		m_info.pNext = nullptr;
		m_info.flags = 0;

		vbl::printError(
			vbl::createShaderModule(&m_info.module, device, code.data(), code.size() * sizeof(uint32_t)),
			"Failed to create shader module"
		);
        m_info.pName = "main";
        m_info.stage = stage;
        m_info.pSpecializationInfo = nullptr;

        m_pCompiler = new spirv_cross::Compiler(code.data(), code.size());


		auto res = m_pCompiler->get_shader_resources();
		addResources(res.uniform_buffers, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr);
		
		addResources(res.sampled_images, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nullptr);
		addResources(res.separate_images, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, nullptr);

		addResources(res.storage_images, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, nullptr);
		addResources(res.storage_buffers, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, nullptr);

		addResources(res.subpass_inputs, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, nullptr);


		// TODO ...

		for (auto& p : res.push_constant_buffers) {
			size_t size = m_pCompiler->get_declared_struct_size(m_pCompiler->get_type(p.type_id));
			
			VkPushConstantRange range = {};
			range.size = size;
			range.offset = 0; // Modify when merged with other shader stage
			range.stageFlags = stage;
			m_pushConstantRanges.push_back(range);
		}


		if (stage == VK_SHADER_STAGE_VERTEX_BIT) {
			getVertexInput(res.stage_inputs);
		}

	}

	Shader::~Shader() {
		vkDestroyShaderModule(m_device, m_info.module, nullptr);
		delete m_pCompiler;
	}

	std::vector<uint32_t> Shader::readCode(const char* path) {
		std::ifstream file(path, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file " + std::string(path));
		}
		const size_t fileSize = file.tellg();
		file.seekg(0);
		std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
		file.read((char*)buffer.data(), fileSize);
		file.close();

		return std::move(buffer);
	}

	VkShaderStageFlagBits Shader::getStage() const {
		return m_stage;
	}

	const std::vector<DescriptorSetLayout>& Shader::getDescriptorSetLayouts() const {
		return m_descriptorSets;
	}

	const std::vector<VkPushConstantRange>& Shader::getPushConstantRanges() const {
		return m_pushConstantRanges;
	}

	VkPipelineShaderStageCreateInfo Shader::getInfo() const {
		return m_info;
	}

	const std::vector<VkVertexInputAttributeDescription>& Shader::getVertexAttributes() const {
		return m_vertexAttributes;
	}

	const std::vector<VkVertexInputBindingDescription>& Shader::getVertexBindings() const {
		return m_vertexBindings;
	}
	
}
