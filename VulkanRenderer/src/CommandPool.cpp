#include "pch.h"
#include "internal/CommandPool.hpp"

#include "internal/debugFunctions.hpp"

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
		m_queueFamilyIndex = queueFamily;
	}

	void CommandPool::destroy() {
		m_device.destroyCommandPool(m_commandPool);
	}

	CommandBufferSet CommandPool::allocateCommandBufferSet(const std::vector<vk::Queue>& queues, vk::CommandBufferLevel level) {
		return CommandBufferSet(m_device, m_commandPool, queues, m_queueFamilyIndex, level);
	}

	CommandBufferSet CommandPool::allocateOneTimeCommandBuffer(vk::Queue queue, vk::CommandBufferLevel level) {
		return CommandBufferSet(m_device, m_commandPool, { queue }, m_queueFamilyIndex, level);
	}
	
	CommandBufferSet::CommandBufferSet()
		: m_currentBufferIndex(-1)
		, m_lastBufferIndex(-1)
	{
	}

	CommandBufferSet::CommandBufferSet(vk::Device device, vk::CommandPool commandPool, const std::vector<vk::Queue>& queues, uint32_t queueFamilyIndex, vk::CommandBufferLevel level)
		: m_currentBufferIndex(0)
		, m_lastBufferIndex(-1)
		, m_queues(queues)
		, m_queueFamilyIndex(queueFamilyIndex)
		, m_device(device)
		, m_commandPool(commandPool)
	{
		create(device, commandPool, queues, level);
	}

	void CommandBufferSet::create(vk::Device device, vk::CommandPool commandPool, const std::vector<vk::Queue>& queues, vk::CommandBufferLevel level)
	{
		m_buffers = device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{
			.commandPool = commandPool,
			.level = level,
			.commandBufferCount = (uint32_t)queues.size(),
		});
	}

	void CommandBufferSet::destroy() {
		m_device.freeCommandBuffers(m_commandPool, m_buffers);
		m_currentBufferIndex = 0;
		m_lastBufferIndex = -1;
		m_buffers.clear();
	}

	void CommandBufferSet::begin(vk::CommandBufferUsageFlags usageFlags, vk::CommandBufferInheritanceInfo* inheritanceinfo) {
		vk::CommandBuffer& buffer = m_buffers.at(m_currentBufferIndex);

		buffer.begin(vk::CommandBufferBeginInfo{
			.flags = usageFlags,
			.pInheritanceInfo = inheritanceinfo,
		});
	}
	
	void CommandBufferSet::end() {
		if (m_currentBufferIndex == -1)
			throw std::runtime_error("Buffer index was -1 : Forgot to call begin");
		m_buffers[m_currentBufferIndex].end();
		m_lastBufferIndex = m_currentBufferIndex;
		m_currentBufferIndex = (m_currentBufferIndex + 1) % m_buffers.size();
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
		m_queues[m_lastBufferIndex].submit(info, fence);
	}

	void CommandBufferSet::present(vk::Semaphore waitSempahore, vk::SwapchainKHR swapchain, uint32_t imageIndex) {
		vk::PresentInfoKHR info;
		info.setSwapchains(swapchain);
		info.setImageIndices(imageIndex);
		info.setWaitSemaphores(waitSempahore);
		m_queues[m_lastBufferIndex].presentKHR(info), "Failed to present image" + std::to_string(imageIndex);

	}

	void CommandBufferSet::present(const std::vector<vk::Semaphore>& waitSempahores, vk::SwapchainKHR swapchain, uint32_t imageIndex) {
		vk::PresentInfoKHR info;
		info.setSwapchains(swapchain);
		info.setImageIndices(imageIndex);
		info.setWaitSemaphores(waitSempahores);

		m_queues[m_lastBufferIndex].presentKHR(info), "Failed to present image" + std::to_string(imageIndex);
	}

	vk::CommandBuffer CommandBufferSet::getBuffer(uint32_t index) const {
		if (index == -1) index = m_currentBufferIndex;
		if (index == -1)
			throw std::runtime_error("Buffer index was -1 : Forgot to call begin");
		return m_buffers[index];
	}

	uint32_t CommandBufferSet::getBufferIndex() const {
		return m_currentBufferIndex;
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
