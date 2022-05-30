#include "pch.h"
#include "CommandPool.hpp"

namespace sa {
	
	void CommandPool::create(vk::Device device, uint32_t queueFamily, vk::Queue queue, vk::CommandPoolCreateFlags flags) {
		m_device = device;

		if (m_commandPool) {
			destroy();
		}
		m_commandPool = device.createCommandPool({ 
			.flags = flags,
			.queueFamilyIndex = queueFamily
		});
		m_queue = queue;
		m_queueFamilyIndex = queueFamily;
	}

	void CommandPool::destroy() {
		m_device.destroyCommandPool(m_commandPool);
	}

	CommandBufferSet CommandPool::allocateCommandBufferSet(uint32_t count, vk::CommandBufferLevel level) {
		return CommandBufferSet(m_device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{
			.commandPool = m_commandPool,
			.level = level,
			.commandBufferCount = count,
		}), m_queue, m_queueFamilyIndex);
	}
	
	CommandBufferSet::CommandBufferSet()
		: m_currentBufferIndex(-1)
		, m_lastBufferIndex(-1)
	{
	}

	CommandBufferSet::CommandBufferSet(const std::vector<vk::CommandBuffer>& buffers, vk::Queue queue, uint32_t queueFamilyIndex)
		: m_buffers(buffers) 
		, m_currentBufferIndex(-1)
		, m_lastBufferIndex(-1)
		, m_targetQueue(queue)
		, m_queueFamilyIndex(queueFamilyIndex)
	{

	}

	void CommandBufferSet::begin(int bufferIndex, vk::CommandBufferUsageFlags usageFlags) {
		if (m_currentBufferIndex != -1)
			throw std::runtime_error("Buffer index was not -1 : Forgot to call end");

		vk::CommandBuffer& buffer = m_buffers.at(bufferIndex);
		buffer.begin(vk::CommandBufferBeginInfo{
			.flags = usageFlags,
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

	bool CommandBufferSet::isRecording() const {
		return m_currentBufferIndex != -1;
	}

	void CommandBufferSet::submit(vk::Fence fence, vk::Semaphore signalSemaphore, vk::Semaphore waitSemaphore) {
		if (m_lastBufferIndex == -1)
			throw std::runtime_error("Buffer was never recorded to");

		vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

		vk::SubmitInfo info{
			.waitSemaphoreCount = (waitSemaphore) ? 1ui32 : 0ui32,
			.pWaitSemaphores = (waitSemaphore) ? &waitSemaphore : nullptr,
			.pWaitDstStageMask = &waitStage,
			.commandBufferCount = 1,
			.pCommandBuffers = &m_buffers[m_lastBufferIndex],
			.signalSemaphoreCount = (signalSemaphore) ? 1ui32 : 0ui32,
			.pSignalSemaphores = (signalSemaphore)? &signalSemaphore : nullptr,
		};
		m_targetQueue.submit(info, fence);
	}

	vk::CommandBuffer CommandBufferSet::getBuffer() const {
		if (m_currentBufferIndex == -1)
			throw std::runtime_error("Buffer index was -1 : Forgot to call begin");
		return m_buffers[m_currentBufferIndex];
	}

	uint32_t CommandBufferSet::getBufferIndex() const {
		return m_currentBufferIndex;
	}

	vk::Queue CommandBufferSet::getTargetQueue() const {
		return m_targetQueue;
	}

	uint32_t CommandBufferSet::getQueueFamilyIndex() const {
		return m_queueFamilyIndex;
	}
	
	uint32_t CommandBufferSet::getBufferCount() const {
		return (uint32_t)m_buffers.size();
	}
	
	bool CommandBufferSet::isValid() const {
		return m_buffers.size() > 0;
	}
}
