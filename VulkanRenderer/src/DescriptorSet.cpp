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

}
