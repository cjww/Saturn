#include "pch.h"
#include "CommandPool.hpp"

namespace sa {
	
	void CommandPool::create(vk::Device device, uint32_t queueFamily, vk::CommandPoolCreateFlags flags) {
		m_device = device;

		if (m_commandPool) {
			destroy();
		}
		m_commandPool = device.createCommandPool({ 
			.flags = flags,
			.queueFamilyIndex = queueFamily
		});
	}

	void CommandPool::destroy() {
		m_device.destroyCommandPool(m_commandPool);
	}

	CommandBufferSet CommandPool::allocateCommandBufferSet(uint32_t count, vk::CommandBufferLevel level) {
		return CommandBufferSet(m_device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{
			.commandPool = m_commandPool,
			.level = level,
			.commandBufferCount = count,
		}));
	}
	
	CommandBufferSet::CommandBufferSet()
		: m_currentBufferIndex(-1)
		, m_lastBufferIndex(-1)
	{
	}

	CommandBufferSet::CommandBufferSet(const std::vector<vk::CommandBuffer>& buffers)
		: m_buffers(buffers) 
		, m_currentBufferIndex(-1)
		, m_lastBufferIndex(-1)
	{

	}

	void CommandBufferSet::begin(int bufferIndex) {
		if (m_currentBufferIndex != -1)
			throw std::runtime_error("Buffer index was not -1 : Forgot to call end");

		vk::CommandBuffer& buffer = m_buffers.at(bufferIndex);
		buffer.begin(vk::CommandBufferBeginInfo{
			.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
			.pInheritanceInfo = nullptr,
		});
		m_currentBufferIndex = bufferIndex;
	}
	
	void CommandBufferSet::end() {
		if (m_currentBufferIndex == -1)
			throw std::runtime_error("Buffer index was -1 : Forgot to call begin");
		m_buffers[m_currentBufferIndex].end();
		m_lastBufferIndex = m_currentBufferIndex;
		m_currentBufferIndex = -1;
	}

	void CommandBufferSet::submit(vk::Queue queue, vk::Fence fence, vk::Semaphore signalSemaphore, vk::Semaphore waitSemaphore) {
		if (m_lastBufferIndex == -1)
			throw std::runtime_error("Buffer was never recorder to");

		vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

		vk::SubmitInfo info{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &waitSemaphore,
			.pWaitDstStageMask = &waitStage,
			.commandBufferCount = 1,
			.pCommandBuffers = &m_buffers[m_lastBufferIndex],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &signalSemaphore,
		};
		queue.submit(info, fence);
	}

	vk::CommandBuffer CommandBufferSet::getBuffer() const {
		if (m_currentBufferIndex == -1)
			throw std::runtime_error("Buffer index was -1 : Forgot to call begin");
		return m_buffers[m_currentBufferIndex];
	}

	uint32_t CommandBufferSet::getBufferIndex() const {
		return m_currentBufferIndex;
	}
}
