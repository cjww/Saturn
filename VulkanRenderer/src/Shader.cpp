#include "pch.h"
#include "Resources/Shader.hpp"

namespace sa {
	void Shader::addResources(const spirv_cross::SmallVector<spirv_cross::Resource>& resources, vk::DescriptorType type, vk::Sampler* immutableSamplers) {
		for (auto& b : resources) {
			vk::DescriptorSetLayoutBinding layoutBinding = {};
			uint32_t set = m_pCompiler->get_decoration(b.id, spv::Decoration::DecorationDescriptorSet);
			layoutBinding.binding = m_pCompiler->get_decoration(b.id, spv::Decoration::DecorationBinding);
			size_t size = 0;
			if (type != vk::DescriptorType::eSampledImage && 
				type != vk::DescriptorType::eCombinedImageSampler &&
				type != vk::DescriptorType::eInputAttachment &&
				type != vk::DescriptorType::eStorageImage) {
				size = m_pCompiler->get_declared_struct_size(m_pCompiler->get_type(b.type_id));
			}
			layoutBinding.stageFlags = m_stage;

			auto t = m_pCompiler->get_type(b.type_id);

			layoutBinding.descriptorCount = (t.array.size() > 0) ? t.array[0] : 1;
			layoutBinding.descriptorType = type;
			layoutBinding.pImmutableSamplers = immutableSamplers;

			vk::WriteDescriptorSet write = {
				.dstBinding = layoutBinding.binding,
				.descriptorCount = layoutBinding.descriptorCount,
				.descriptorType = layoutBinding.descriptorType,
			};

			// Needs to be set when writing
			write.pImageInfo = nullptr;
			write.pTexelBufferView = nullptr;
			write.pBufferInfo = nullptr;
			write.dstArrayElement = 0;
			write.dstSet = nullptr;

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
		std::unordered_map<uint32_t, vk::VertexInputAttributeDescription> attribs;

		for (auto& input : resources) {
			auto type = m_pCompiler->get_type(input.type_id);
			uint32_t location = m_pCompiler->get_decoration(input.id, spv::Decoration::DecorationLocation);
			uint32_t binding = m_pCompiler->get_decoration(input.id, spv::Decoration::DecorationBinding);
			vk::Format format = vk::Format::eUndefined;
			if (type.basetype == spirv_cross::SPIRType::BaseType::UInt) {
				format = vk::Format::eR32Uint;
				sizes[location] = sizeof(unsigned int);
			}
			else if (type.basetype == spirv_cross::SPIRType::BaseType::Int || type.basetype == spirv_cross::SPIRType::BaseType::Boolean) {
				format = vk::Format::eR32Sint;
				sizes[location] = sizeof(int);
			}
			else {
				int i = type.vecsize;
				switch (i) {
				case 1:
					format = vk::Format::eR32Sfloat;
					break;
				case 2:
					format = vk::Format::eR32G32Sfloat;
					break;
				case 3:
					format = vk::Format::eR32G32B32Sfloat;
					break;
				case 4:
					format = vk::Format::eR32G32B32A32Sfloat;
					break;
				}
				sizes[location] = sizeof(float) * i;

			}

			if (format == vk::Format::eUndefined) {
				throw std::runtime_error("Undefined vertex input format");
			}

			vk::VertexInputAttributeDescription attrib = {
				.location = location,
				.binding = binding,
				.format = format,
			};
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
			vk::VertexInputBindingDescription b = {
				.binding = 0,
				.stride = size,
				.inputRate = vk::VertexInputRate::eVertex,
			};

			m_vertexBindings = { b };
		}
	}
	
	Shader::Shader(vk::Device device, const char* path, vk::ShaderStageFlagBits stage) {
		create(device, path, stage);
	}

	void Shader::create(vk::Device device, const char* path, vk::ShaderStageFlagBits stage)
	{
		m_device = device;
		m_stage = stage;
		
		load(path);
	}

	void Shader::destroy() {
		m_device.destroyShaderModule(m_info.module);
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

	void Shader::load(const char* path) {

		auto code = readCode(path);

		if (m_info.module) {
			m_device.destroyShaderModule(m_info.module);
			m_info.module = nullptr;
		}

		vk::ShaderModuleCreateInfo shaderInfo{
			.codeSize = code.size() * sizeof(uint32_t),
			.pCode = (const uint32_t*)code.data(),
		};

		m_info.module = m_device.createShaderModule(shaderInfo);

		m_info.setPName("main");
		m_info.setStage(m_stage);

		m_pCompiler = std::make_shared<spirv_cross::Compiler>(code.data(), code.size());
		for (auto ext : m_pCompiler->get_declared_extensions()) {
			DEBUG_LOG_INFO("Shader uses vulkan extension: ", ext);
		}

		m_descriptorSets.clear();
		auto res = m_pCompiler->get_shader_resources();

		addResources(res.uniform_buffers, vk::DescriptorType::eUniformBuffer, nullptr);

		addResources(res.sampled_images, vk::DescriptorType::eCombinedImageSampler, nullptr);
		addResources(res.separate_images, vk::DescriptorType::eSampledImage, nullptr);

		addResources(res.storage_images, vk::DescriptorType::eStorageImage, nullptr);
		addResources(res.storage_buffers, vk::DescriptorType::eStorageBuffer, nullptr);

		addResources(res.subpass_inputs, vk::DescriptorType::eInputAttachment, nullptr);
		// TODO ...



		m_pushConstantRanges.clear();
		for (auto& p : res.push_constant_buffers) {
			size_t size = m_pCompiler->get_declared_struct_size(m_pCompiler->get_type(p.type_id));

			vk::PushConstantRange range = {
				.stageFlags = m_stage,
				.offset = 0, // Modify when merged with other shader stage
				.size = static_cast<uint32_t>(size),
			};
			m_pushConstantRanges.push_back(range);
		}

		m_vertexAttributes.clear();
		m_vertexBindings.clear();
		if (m_stage == vk::ShaderStageFlagBits::eVertex) {
			getVertexInput(res.stage_inputs);
		}
	}

	vk::ShaderStageFlagBits Shader::getStage() const {
		return m_stage;
	}

	const std::vector<DescriptorSetLayout>& Shader::getDescriptorSetLayouts() const {
		return m_descriptorSets;
	}

	const std::vector<vk::PushConstantRange>& Shader::getPushConstantRanges() const {
		return m_pushConstantRanges;
	}

	vk::PipelineShaderStageCreateInfo Shader::getInfo() const {
		return m_info;
	}

	const std::vector<vk::VertexInputAttributeDescription>& Shader::getVertexAttributes() const {
		return m_vertexAttributes;
	}

	const std::vector<vk::VertexInputBindingDescription>& Shader::getVertexBindings() const {
		return m_vertexBindings;
	}
	
}
