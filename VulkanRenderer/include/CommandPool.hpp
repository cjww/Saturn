#pragma once

namespace sa {

	class CommandBufferSet {
	private:
		std::vector<vk::CommandBuffer> m_buffers;
		int m_lastBufferIndex;
		int m_currentBufferIndex;
		vk::Queue m_targetQueue;

	public:
		CommandBufferSet();
		CommandBufferSet(const std::vector<vk::CommandBuffer>& buffers, vk::Queue queue);
		void begin(int bufferIndex, vk::CommandBufferUsageFlags usageFlags);
		void end();
		
		bool isRecording() const;

		void submit(vk::Fence fence = {}, vk::Semaphore signalSemaphore = {}, vk::Semaphore waitSemaphore = {});

		vk::CommandBuffer getBuffer() const;
		uint32_t getBufferIndex() const;

		vk::Queue getTargetQueue() const;

		uint32_t getBufferCount() const;

		bool isValid() const;
	};

	class CommandPool {
	private:
		vk::CommandPool m_commandPool;
		vk::Device m_device;
		vk::Queue m_queue;
	public:
		CommandPool() = default;

		void create(vk::Device device, uint32_t queueFamily, vk::Queue queue, vk::CommandPoolCreateFlags flags);
		void destroy();

		CommandBufferSet allocateCommandBufferSet(uint32_t count, vk::CommandBufferLevel level);
	};
}