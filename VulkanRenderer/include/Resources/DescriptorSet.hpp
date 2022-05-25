#pragma once

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

	public:
		void create(
			vk::Device device,
			vk::DescriptorPool descriptorPool,
			uint32_t count,
			DescriptorSetLayout info,
			vk::DescriptorSetLayout layout,
			uint32_t setIndex);
		void destroy();
	};
}
