#include "pch.h"
#include "FramebufferSet.hpp"

namespace sa{
	FramebufferSet::FramebufferSet() {

	}

	FramebufferSet::FramebufferSet(const std::vector<vk::Framebuffer>& buffers) 
		: m_buffers(buffers)
	{

	}

	void FramebufferSet::bind(int bufferIndex) {
		//m_buffers.at(bufferIndex);

	}

}