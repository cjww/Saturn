#include "pch.h"
#include "Resources/DynamicBuffer.hpp"

namespace sa{

	void DynamicBuffer::setBufferIndex(uint32_t index) {
		m_currentBufferIndex = index;
	}


	DynamicBuffer::DynamicBuffer() 
		: m_currentBufferIndex(0)
	{

	}

	DynamicBuffer::DynamicBuffer(VulkanCore* pCore, BufferType type, uint32_t bufferCount, size_t size, void* initialData) 
		: DynamicBuffer() 
	{
		m_buffers.resize(bufferCount);
		for (uint32_t i = 0; i < bufferCount; i++) {
			m_buffers[i] = Buffer(pCore, type, size, initialData);
		}
	}

	void DynamicBuffer::destroy() {
		for (auto& buffer : m_buffers) {
			buffer.destroy();
		}
		m_currentBufferIndex = 0;
	}

	Buffer& DynamicBuffer::getBuffer(uint32_t index) {
		if (index == -1) {
			index = m_currentBufferIndex;
			m_currentBufferIndex = (m_currentBufferIndex + 1) % m_buffers.size();
		}
		return m_buffers.at(index);
	}

	const Buffer& DynamicBuffer::getBuffer(uint32_t index) const {
		if (index == -1) {
			index = m_currentBufferIndex;
		}
		return m_buffers.at(index);
	}

	const Buffer& DynamicBuffer::getCurrentBuffer() const {
		return getBuffer(m_currentBufferIndex);
	}

	void DynamicBuffer::manualIncrement() {
		m_currentBufferIndex = (m_currentBufferIndex + 1) % m_buffers.size();
	}

	uint32_t DynamicBuffer::getBufferCount() const {
		return m_buffers.size();
	}

	BufferType DynamicBuffer::getType() const {
		return m_buffers.front().getType();
	}

	void DynamicBuffer::write(void* data, size_t size, int offset) {
		getBuffer(m_currentBufferIndex).write(data, size, offset);
	}

	void DynamicBuffer::append(void* data, size_t size, int alignment) {
		getBuffer(m_currentBufferIndex).append(data, size, alignment);
	}

	void DynamicBuffer::resize(size_t newSize, BufferResizeFlags resizeFlags) {
		getBuffer(m_currentBufferIndex).resize(newSize, resizeFlags);
	}

	void DynamicBuffer::reserve(size_t capacity, BufferResizeFlags resizeFlags) {
		getBuffer(m_currentBufferIndex).reserve(capacity, resizeFlags);
	}

	void* DynamicBuffer::data() {
		return getBuffer(m_currentBufferIndex).data();
	}

	void DynamicBuffer::clear() {
		getBuffer(m_currentBufferIndex).clear();
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
