#include "pch.h"
#include "Resources/DynamicBuffer.hpp"
#include "Renderer.hpp"
#include "internal\VulkanCore.hpp"

namespace sa{

	void DynamicBuffer::setBufferIndex(uint32_t index) {
		m_currentBufferIndex = index;
	}


	DynamicBuffer::DynamicBuffer() 
		: m_currentBufferIndex(0)
	{

	}

	DynamicBuffer::DynamicBuffer(BufferType type, size_t size, void* initialData) 
		: DynamicBuffer() 
	{
		create(type, size, initialData);
	}

	void DynamicBuffer::create(BufferType type, size_t size, void* initialData) {
		uint32_t bufferCount = Renderer::Get().getCore()->getQueueCount();
		m_buffers.resize(bufferCount);
		for (uint32_t i = 0; i < bufferCount; i++) {
			m_buffers[i] = Buffer(type, size, initialData);
		}
	}

	void DynamicBuffer::destroy() {
		for (auto& buffer : m_buffers) {
			buffer.destroy();
		}
		m_currentBufferIndex = 0;
	}

	
	const sa::Buffer& DynamicBuffer::getBuffer() const {
		return m_buffers.at(m_currentBufferIndex);
	}

	Buffer& DynamicBuffer::getBuffer(uint32_t index) {
		return m_buffers.at(index);
	}

	uint32_t DynamicBuffer::getBufferIndex() const {
		return m_currentBufferIndex;
	}

	uint32_t DynamicBuffer::getPreviousBufferIndex() const {
		return (m_currentBufferIndex + m_buffers.size() - 1) % m_buffers.size();
	}

	uint32_t DynamicBuffer::getNextBufferIndex() const {
		return (m_currentBufferIndex + 1U) % (uint32_t)m_buffers.size();
	}

	void DynamicBuffer::swap() {
		m_currentBufferIndex = (m_currentBufferIndex + 1) % static_cast<uint32_t>(m_buffers.size());
	}

	uint32_t DynamicBuffer::getBufferCount() const {
		return m_buffers.size();
	}

	BufferType DynamicBuffer::getType() const {
		return m_buffers.front().getType();
	}

	void DynamicBuffer::write(void* data, size_t size, int offset) {
		m_buffers[m_currentBufferIndex].write(data, size, offset);
	}

	void DynamicBuffer::append(void* data, size_t size, int alignment) {
		m_buffers[m_currentBufferIndex].append(data, size, alignment);
	}

	void DynamicBuffer::copy(const Buffer& other) {
		m_buffers[m_currentBufferIndex].copy(other);
	}

	void DynamicBuffer::resize(size_t newSize, BufferResizeFlags resizeFlags) {
		m_buffers[m_currentBufferIndex].resize(newSize, resizeFlags);
	}

	void DynamicBuffer::reserve(size_t capacity, BufferResizeFlags resizeFlags) {
		m_buffers[m_currentBufferIndex].reserve(capacity, resizeFlags);
	}

	void* DynamicBuffer::data() {
		return getBuffer(m_currentBufferIndex).data();
	}

	void* DynamicBuffer::data(uint32_t offset) {
		return getBuffer(m_currentBufferIndex).data(offset);
	}

	void DynamicBuffer::clear() {
		m_buffers[m_currentBufferIndex].clear();
	}

	bool DynamicBuffer::isValid() const {
		for (const auto& buffer : m_buffers) {
			if (!buffer.isValid()) return false;
		}
		return true;
	}

	DynamicBuffer::operator const Buffer& () {
		return getBuffer();
	}


}
