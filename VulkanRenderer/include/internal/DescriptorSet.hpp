#pragma once

#include "Resources\Texture.hpp"
#include "Resources\Buffer.hpp"
#include "DeviceMemoryManager.hpp"
#include "DescriptorSetStructs.h"

#define MAX_VARIABLE_DESCRIPTOR_COUNT 1024
#define VARIABLE_DESCRIPTOR_COUNT 0

namespace sa {

	class DescriptorSet {
	private:
		vk::Device m_device;
		vk::DescriptorPool m_descriptorPool;
		std::vector<vk::DescriptorSet> m_descriptorSets;
		std::unordered_map<uint32_t, vk::WriteDescriptorSet> m_writes;
		uint32_t m_setIndex;

		void update(uint32_t binding, uint32_t arrayIndex, uint32_t indexToUpdate);

	public:

		void create(
			vk::Device device,
			vk::DescriptorPool descriptorPool,
			uint32_t count,
			DescriptorSetLayoutInfo info,
			vk::DescriptorSetLayout layout,
			uint32_t setIndex);
		void destroy();

		void update(uint32_t binding, vk::Buffer buffer, vk::DeviceSize bufferSize, vk::DeviceSize bufferOffset, vk::BufferView* pView, uint32_t indexToUpdate);
		void update(uint32_t binding, vk::ImageView view, vk::ImageLayout layout, vk::Sampler* pSampler, uint32_t indexToUpdate);
		
		void update(uint32_t binding, uint32_t firstElement, const Texture* textures, uint32_t textureCount, vk::Sampler* pSampler, uint32_t indexToUpdate);
		void update(uint32_t binding, uint32_t firstElement, const Buffer* buffers, uint32_t bufferCount, uint32_t indexToUpdate);

		vk::DescriptorSet getSet(uint32_t index) const;

		uint32_t getSetIndex() const;

		vk::DescriptorType getDescriptorType(uint32_t binding) const;

	};
}
