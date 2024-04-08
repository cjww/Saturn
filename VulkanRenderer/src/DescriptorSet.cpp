#include "pch.h"
#include "internal/DescriptorSet.hpp"

namespace sa {
	
	void DescriptorSet::create(vk::Device device, vk::DescriptorPool descriptorPool, uint32_t count, DescriptorSetLayoutInfo info, vk::DescriptorSetLayout layout, uint32_t setIndex) {
		m_device = device;
		m_descriptorPool = descriptorPool;

		std::vector<vk::DescriptorSetLayout> layouts(count, layout);
		vk::DescriptorSetAllocateInfo allocInfo{
			.descriptorPool = m_descriptorPool,
			.descriptorSetCount = static_cast<uint32_t>(count),
			.pSetLayouts = layouts.data(),
		};

		vk::DescriptorSetVariableDescriptorCountAllocateInfo varDescCountAllocInfo;
		std::vector<uint32_t> counts;

		auto it = std::max_element(info.bindings.begin(), info.bindings.end(), [](const auto& highest, const auto& next) { return highest.binding < next.binding; });
		if (it != info.bindings.end()) {
			if (it->descriptorCount == VARIABLE_DESCRIPTOR_COUNT) { // has a variable descriptor
				counts.resize(count, MAX_VARIABLE_DESCRIPTOR_COUNT);
				varDescCountAllocInfo.setDescriptorCounts(counts);
				allocInfo.setPNext(&varDescCountAllocInfo);
			}
		}
		
		m_descriptorSets = m_device.allocateDescriptorSets(allocInfo);

		m_setIndex = setIndex;

		for (uint32_t i = 0; i < (uint32_t)info.bindings.size(); i++) {
			uint32_t binding = info.bindings[i].binding;
			
			m_writes[binding].dstBinding = binding;
			m_writes[binding].descriptorType = (vk::DescriptorType)info.bindings[i].type;
			m_writes[binding].descriptorCount = info.bindings[i].descriptorCount;
		}

	}
	

	void DescriptorSet::update(uint32_t binding, uint32_t arrayIndex, uint32_t indexToUpdate) {
		m_writes[binding].dstArrayElement = arrayIndex;
		
		if (indexToUpdate == UINT32_MAX) {
			m_device.waitIdle();
			std::vector<vk::WriteDescriptorSet> writes;
			for (uint32_t i = 0; i < m_descriptorSets.size(); i++) {
				m_writes[binding].dstSet = m_descriptorSets[i];
				writes.push_back(m_writes[binding]);
			}
			m_device.updateDescriptorSets(writes, nullptr);
		}
		else {
			m_writes[binding].dstSet = m_descriptorSets[indexToUpdate];
			m_device.updateDescriptorSets(m_writes[binding], nullptr);
		}
	}

	void DescriptorSet::destroy() {
		if (!m_descriptorPool || m_descriptorSets.empty())
			return;
		m_device.waitIdle();
		m_device.freeDescriptorSets(m_descriptorPool, m_descriptorSets);
			
		m_descriptorPool = VK_NULL_HANDLE;
		m_descriptorSets.clear();
	}

	void DescriptorSet::update(uint32_t binding, vk::Buffer buffer, vk::DeviceSize bufferSize, vk::DeviceSize bufferOffset, vk::BufferView* pView, uint32_t indexToUpdate) {

		if (!m_writes.count(binding)) {
			SA_DEBUG_LOG_ERROR("Binding ", binding, " does not exist!");
			throw std::runtime_error("Binding " + std::to_string(binding) + " does not exist!");
		}

		vk::DescriptorBufferInfo bufferInfo = {
			.buffer = buffer,
			.offset = bufferOffset,
			.range = bufferSize,
		};

		m_writes[binding].setBufferInfo(bufferInfo);
		
		m_writes[binding].setPTexelBufferView(pView);

		update(binding, 0, indexToUpdate);

	}

	void DescriptorSet::update(uint32_t binding, vk::ImageView imageView, vk::ImageLayout layout, vk::Sampler* pSampler, uint32_t indexToUpdate) {
		if (!m_writes.count(binding)) {
			SA_DEBUG_LOG_ERROR("Binding ", binding, " does not exist!");
			throw std::runtime_error("Binding " + std::to_string(binding) + " does not exist!");
		}

		vk::DescriptorImageInfo imageInfo{
			.sampler = (pSampler)? *pSampler : nullptr,
			.imageView = imageView,
			.imageLayout = layout,
		};

		m_writes[binding].setImageInfo(imageInfo);

		update(binding, 0, indexToUpdate);

	}

	void DescriptorSet::update(uint32_t binding, uint32_t firstElement, const Texture* textures, uint32_t textureCount, vk::Sampler* pSampler, uint32_t indexToUpdate) {
		if (textureCount == 0 || textures == nullptr)
			return;

		if (!m_writes.count(binding)) {
			SA_DEBUG_LOG_ERROR("Binding ", binding, " does not exist!");
			throw std::runtime_error("Binding " + std::to_string(binding) + " does not exist!");
		}

		vk::ImageLayout imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		if (getDescriptorType(binding) == vk::DescriptorType::eStorageImage || 
			(textures[0].getUsageFlags() & sa::TextureUsageFlagBits::STORAGE) == sa::TextureUsageFlagBits::STORAGE)
		{
			imageLayout = vk::ImageLayout::eGeneral;
		}

		std::vector<vk::DescriptorImageInfo> imageInfos;
		imageInfos.reserve(textureCount);
		for (uint32_t i = 0; i < textureCount; i++) {
			const Texture& tex = textures[i];
			imageInfos.push_back(vk::DescriptorImageInfo {
				.sampler = (pSampler) ? *pSampler : nullptr,
				.imageView = *tex.getView(),
				.imageLayout = imageLayout,
			});
		}

		m_writes[binding].setImageInfo(imageInfos);

		update(binding, firstElement, indexToUpdate);
	}

	void DescriptorSet::update(uint32_t binding, uint32_t firstElement, const Buffer* buffers, uint32_t bufferCount, uint32_t indexToUpdate) {
		if (bufferCount == 0 || buffers == nullptr)
			return;

		if (!m_writes.count(binding)) {
			SA_DEBUG_LOG_ERROR("Binding ", binding, " does not exist!");
			throw std::runtime_error("Binding " + std::to_string(binding) + " does not exist!");
		}


		std::vector<vk::DescriptorBufferInfo> bufferInfos;
		std::vector<vk::BufferView> bufferViews;
		for (uint32_t i = 0; i < bufferCount; i++) {
			const Buffer& buffer = buffers[i];
			if (buffer.getType() == BufferType::UNIFORM_TEXEL || buffer.getType() == BufferType::STORAGE_TEXEL) {
				bufferViews.push_back(*buffer.getView());
				m_writes[binding].setTexelBufferView(bufferViews);
			}
			else {
				const DeviceBuffer* pBuf = buffer;
				bufferInfos.push_back(vk::DescriptorBufferInfo {
					.buffer = pBuf->buffer,
					.offset = 0,
					.range = pBuf->size,
				});
				m_writes[binding].setBufferInfo(bufferInfos);
			}
		}

		update(binding, firstElement, indexToUpdate);
	}

	vk::DescriptorSet DescriptorSet::getSet(uint32_t index) const {
		return m_descriptorSets.at(index);
	}

	uint32_t DescriptorSet::getSetIndex() const {
		return m_setIndex;
	}

	vk::DescriptorType DescriptorSet::getDescriptorType(uint32_t binding) const {
		return m_writes.at(binding).descriptorType;
	}

}
