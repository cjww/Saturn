#include "pch.h"
#include "Resources/FramebufferSet.hpp"

namespace sa{
	FramebufferSet::FramebufferSet() 
		: m_extent({0, 0})
	{
	}

	FramebufferSet::FramebufferSet(vk::Device device, const std::vector<vk::Framebuffer>& buffers, Extent extent) {
		create(device, buffers, extent);
	}


	void FramebufferSet::create(vk::Device device, const std::vector<vk::Framebuffer>& buffers, Extent extent) {
		m_device = device;
		m_buffers = buffers;
		m_extent = extent;
	}

	void FramebufferSet::destroy() {
		for (auto& framebuffer : m_buffers) {
			m_device.destroyFramebuffer(framebuffer);
		}
	}

	vk::Framebuffer FramebufferSet::getBuffer(uint32_t index) const {
		return m_buffers.at(index);
	}


	Extent FramebufferSet::getExtent() const {
		return m_extent;
	}

}