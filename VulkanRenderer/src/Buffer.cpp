#include "pch.h"
#include "Resources/Buffer.hpp"

#include "VulkanCore.hpp"

namespace sa {
	
	Buffer::Buffer()
		: m_pBuffer(nullptr)
		, m_pCore(nullptr)
	{

	}

	Buffer::Buffer(VulkanCore* pCore, BufferType type, size_t size, size_t elementCount, void* initialData) {
		m_pCore = pCore;
		m_elementCount = elementCount;
		m_type = type;
		
		create(type, size, initialData);
	}

	void Buffer::create(BufferType type, size_t size, void* initialData) {
		if (size == 0)
			size = 1;

		void* mem = malloc(size);
		if (!initialData) {
			memset(mem, 0, size);
			initialData = mem;
		}
		//init(size, initialData);
		switch (type) {
		case BufferType::VERTEX:
			
			break;
		case BufferType::INDEX:
			break;
		case BufferType::UNIFORM:
			break;
		case BufferType::STORAGE:
			break;
		}

		free(mem);

		
	}

	void Buffer::destroy() {
	
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
			m_pCore->destroyBuffer(m_pBuffer);
		}
		
		create(m_type, newSize, data);
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