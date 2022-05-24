#pragma once

namespace sa {

	class CommandBufferSet {
	private:
		std::vector<vk::CommandBuffer> m_buffers;
		int m_lastBufferIndex;
		int m_currentBufferIndex;

	public:
		CommandBufferSet();
		CommandBufferSet(const std::vector<vk::CommandBuffer>& buffers);
		void begin(int bufferIndex);
		void end();
		
		void submit(vk::Queue queue, vk::Fence fence = {}, vk::Semaphore signalSemaphore = {}, vk::Semaphore waitSemaphore = {});

		vk::CommandBuffer getBuffer() const;
		uint32_t getBufferIndex() const;

	};

	class CommandPool {
	private:
		vk::CommandPool m_commandPool;
		vk::Device m_device;
	public:
		CommandPool() = default;

		void create(vk::Device device, uint32_t queueFamily, vk::CommandPoolCreateFlags flags);
		void destroy();

		CommandBufferSet allocateCommandBufferSet(uint32_t count, vk::CommandBufferLevel level);
	};
}