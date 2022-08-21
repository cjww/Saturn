#pragma once

#include "Texture.hpp"
#include "Buffer.hpp"
#include "DeviceMemoryManager.hpp"

#define MAX_VARIABLE_DESCRIPTOR_COUNT 1024

namespace sa {

	struct DescriptorSetLayout {
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		std::vector<vk::WriteDescriptorSet> writes;
		std::vector<size_t> sizes;
	};

	class DescriptorSet {
	private:
		vk::Device m_device;
		vk::DescriptorPool m_descriptorPool;
		std::vector<vk::DescriptorSet> m_descriptorSets;
		std::vector<vk::WriteDescriptorSet> m_writes;
		uint32_t m_setIndex;

		void update(uint32_t binding, uint32_t arrayIndex, uint32_t indexToUpdate);

	public:
		void create(
			vk::Device device,
			vk::DescriptorPool descriptorPool,
			uint32_t count,
			DescriptorSetLayout info,
			vk::DescriptorSetLayout layout,
			uint32_t setIndex);
		void destroy();

		void update(uint32_t binding, vk::Buffer buffer, vk::DeviceSize bufferSize, vk::DeviceSize bufferOffset, vk::BufferView* pView, uint32_t indexToUpdate);
		void update(uint32_t binding, vk::ImageView view, vk::ImageLayout layout, vk::Sampler* pSampler, uint32_t indexToUpdate);
		void update(uint32_t binding, uint32_t firstElement, const std::vector<Texture>& textures, vk::Sampler* pSampler, uint32_t indexToUpdate);
		void update(uint32_t binding, uint32_t firstElement, const std::vector<Buffer>& buffers, uint32_t indexToUpdate);

		vk::DescriptorSet getSet(uint32_t index) const;

		uint32_t getSetIndex() const;

		vk::DescriptorType getDescriptorType(int binding) const;

	};
}
