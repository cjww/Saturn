#pragma once

namespace sa {

	class CommandBufferSet {
	private:
		std::vector<vk::CommandBuffer> m_buffers;
		int m_lastBufferIndex;
		int m_currentBufferIndex;
		std::vector<vk::Queue> m_queues;
		uint32_t m_queueFamilyIndex;

	public:
		CommandBufferSet();
		CommandBufferSet(vk::Device device, vk::CommandPool commandPool, const std::vector<vk::Queue>& queues, uint32_t queueFamilyIndex, vk::CommandBufferLevel level);
		void create(vk::Device device, vk::CommandPool commandPool, const std::vector<vk::Queue>& queues, vk::CommandBufferLevel level);

		void begin(vk::CommandBufferUsageFlags usageFlags);
		void end();
		
		bool isRecording() const;

		void submit(vk::Fence fence = {}, vk::Semaphore signalSemaphore = {}, vk::Semaphore waitSemaphore = {});

		void present(vk::Semaphore waitSempahore, vk::SwapchainKHR swapchain, uint32_t imageIndex);

		vk::CommandBuffer getBuffer() const;
		uint32_t getBufferIndex() const;

		uint32_t getQueueFamilyIndex() const;

		uint32_t getBufferCount() const;

		bool isValid() const;
	};

	class CommandPool {
	private:
		vk::CommandPool m_commandPool;
		vk::Device m_device;
		uint32_t m_queueFamilyIndex;
	public:
		CommandPool() = default;

		void create(vk::Device device, uint32_t queueFamily, vk::CommandPoolCreateFlags flags);
		void destroy();

		CommandBufferSet allocateCommandBufferSet(const std::vector<vk::Queue>& queues, vk::CommandBufferLevel level);
	};
}