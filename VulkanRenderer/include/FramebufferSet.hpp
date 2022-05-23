#pragma once

namespace sa {
	class FramebufferSet {
	private:
		std::vector<vk::Framebuffer> m_buffers;
	public:
		FramebufferSet();
		FramebufferSet(const std::vector<vk::Framebuffer>& buffers);

		void bind(int bufferIndex);

	};
}