#include "pch.h"
#include "Buffer.h"

#include <Graphics\Vulkan\Renderer.hpp>

namespace sa {
	void Buffer::init(size_t size, void* initialData) {
		switch (m_type) {
		case BufferType::VERTEX:
			m_pBuffer = vr::Renderer::get()->createVertexBuffer(size, initialData);
			break;
		case BufferType::INDEX:
			m_pBuffer = vr::Renderer::get()->createIndexBuffer(size, initialData);
			break;
		case BufferType::UNIFORM:
			m_pBuffer = vr::Renderer::get()->createUniformBuffer(size, initialData);
			break;
		case BufferType::STORAGE:
			m_pBuffer = vr::Renderer::get()->createStorageBuffer(size, initialData);
			break;
		}
	}

	Buffer::Buffer(BufferType type, size_t size, size_t elementCount, void* initialData)
	{
		if (size == 0) 
			size = 1;

		void* mem = malloc(size);
		if (!initialData) {
			memset(mem, 0, size);
			initialData = mem;
		}
		m_elementCount = elementCount;
		m_type = type;
		init(size, initialData);

		free(mem);
	}

	void Buffer::resize(size_t newSize)
	{
		void* data = malloc(newSize);
		if (isValid())
		{
			//Save old data
			memcpy(data, m_pBuffer->mappedData, std::min(newSize, getSize()));
		
			int diff = newSize - getSize();
			if (diff > 0) {
				// if space for more initialize it to 0
				memset((char*)data + getSize(), 0, diff);
			}
			vr::Renderer::get()->destroyBuffer(m_pBuffer);
		}
		
		init(newSize, data);
		free(data);
	}

	size_t Buffer::getElementCount() const {
		return m_elementCount;
	}

	size_t Buffer::getSize() const {
		return m_pBuffer->size;
	}

	void sa::Buffer::write(void* data, size_t size, size_t elementCount, int offset) {
		if (!isValid()) {
			DEBUG_LOG_ERROR("Buffer not initialized! Wrote 0 bytes");
			return;
		}
		if (getSize() < size) {
			resize(size);
		}
		memcpy((char*)m_pBuffer->mappedData + offset, data, size);
		m_elementCount = elementCount;
	}

	void* Buffer::data() {
		return m_pBuffer->mappedData;
	}

	bool Buffer::isValid() const {
		return m_pBuffer != nullptr;
	}

}