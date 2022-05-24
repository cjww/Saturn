#pragma once

#include "structs.hpp"

namespace sa {
	class FramebufferSet {
	private:
		std::vector<vk::Framebuffer> m_buffers;
		Extent m_extent;
		vk::Device m_device;

	public:
		FramebufferSet();
		FramebufferSet(vk::Device device, const std::vector<vk::Framebuffer>& buffers, Extent extent);

		void create(vk::Device device, const std::vector<vk::Framebuffer>& buffers, Extent extent);
		void destroy();


		vk::Framebuffer getBuffer(uint32_t index) const;

		Extent getExtent() const;

	};
}