#include "pch.h"
#include "Resources/DescriptorSet.hpp"

namespace sa {
	void DescriptorSet::create(vk::Device device, vk::DescriptorPool descriptorPool, uint32_t count, DescriptorSetLayout info, vk::DescriptorSetLayout layout, uint32_t setIndex) {
		m_device = device;
		m_descriptorPool = descriptorPool;

		
		std::vector<vk::DescriptorSetLayout> layouts(count, layout);
		vk::DescriptorSetAllocateInfo allocInfo{
			.descriptorPool = m_descriptorPool,
			.descriptorSetCount = static_cast<uint32_t>(count),
			.pSetLayouts = layouts.data(),
		};
		m_descriptorSets = m_device.allocateDescriptorSets(allocInfo);

		m_setIndex = setIndex;

		m_writes.resize(info.bindings.size());
		for (uint32_t i = 0; i < (uint32_t)info.bindings.size(); i++) {
			uint32_t binding = info.bindings[i].binding;
			if (m_writes.size() <= binding) {
				int diff = binding - (m_writes.size() - 1);
				m_writes.resize(m_writes.size() + diff);
			}
			m_writes[binding] = info.writes[i];
		}

	}
	
	void DescriptorSet::destroy() {
		m_device.freeDescriptorSets(m_descriptorPool, m_descriptorSets);
	}

	void DescriptorSet::update(uint32_t binding, vk::Buffer buffer, vk::DeviceSize bufferSize, vk::DeviceSize bufferOffset, uint32_t indexToUpdate) {

		if (m_writes.size() <= binding) {
			DEBUG_LOG_ERROR("Binding", binding, "out of bounds!");
			throw std::runtime_error("Binding " + std::to_string(binding) + " out of bounds!");
		}

		vk::DescriptorBufferInfo bufferInfo = {
			.buffer = buffer,
			.offset = bufferOffset,
			.range = bufferSize,
		};

		VkDescriptorImageInfo imageInfo = {};

		m_writes[binding].setBufferInfo(bufferInfo);
		
		/*
		else if (image != nullptr) {
			imageInfo.imageLayout = image->layout;
			imageInfo.imageView = image->view;
			imageInfo.sampler = sampler == nullptr ? VK_NULL_HANDLE : *sampler;
			descriptorSet->writes[binding].pImageInfo = &imageInfo;
			descriptorSet->writes[binding].pBufferInfo = nullptr;
			descriptorSet->writes[binding].pTexelBufferView = nullptr;
		}
		*/

		if (indexToUpdate == UINT32_MAX) {
			std::vector<vk::WriteDescriptorSet> writes;
			for (uint32_t i = 0; i < m_descriptorSets.size(); i++) {
				m_writes[binding].dstSet = m_descriptorSets[i];
				m_writes[binding].dstArrayElement = 0;
				writes.push_back(m_writes[binding]);
			}
			m_device.updateDescriptorSets(writes, nullptr);
		}
		else {
			m_writes[binding].dstSet = m_descriptorSets[indexToUpdate];
			m_writes[binding].dstArrayElement = 0;

			m_device.updateDescriptorSets(m_writes[binding], nullptr);
		}

	}

	vk::DescriptorSet DescriptorSet::getSet(uint32_t index) const {
		return m_descriptorSets.at(index);
	}

	uint32_t DescriptorSet::getSetIndex() const {
		return m_setIndex;
	}

}
