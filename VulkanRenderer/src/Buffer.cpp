#include "pch.h"
#include "Resources/Buffer.hpp"

#include "VulkanCore.hpp"

namespace sa {
	
	std::string to_string(const BufferType& value) {
		switch (value)
		{
		case sa::BufferType::VERTEX:
			return "VERTEX";
		case sa::BufferType::INDEX:
			return "INDEX";
		case sa::BufferType::UNIFORM:
			return "UNIFORM";
		case sa::BufferType::STORAGE:
			return "STORAGE";
		default:
			return "-";
		}
	}

	Buffer::Buffer()
		: m_pBuffer(nullptr)
		, m_pCore(nullptr)
		, m_size(0)
		, m_type(BufferType::VERTEX)
	{

	}

	Buffer::Buffer(VulkanCore* pCore, BufferType type, size_t size, void* initialData) {
		m_pCore = pCore;
		m_type = type;
		m_size = (initialData) ? size : 0;
		create(type, size, initialData);
	}

	void Buffer::create(BufferType type, size_t size, void* initialData) {
		if (size == 0)
			size = 1;

		switch (type) {
		case BufferType::VERTEX:
			m_pBuffer = m_pCore->createBuffer(vk::BufferUsageFlagBits::eVertexBuffer, 
				VMA_MEMORY_USAGE_AUTO, 
				VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
				size, initialData);
			break;
		case BufferType::INDEX:
			m_pBuffer = m_pCore->createBuffer(vk::BufferUsageFlagBits::eIndexBuffer,
				VMA_MEMORY_USAGE_AUTO, 
				VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
				size, initialData);
			break;
		case BufferType::UNIFORM:
			m_pBuffer = m_pCore->createBuffer(vk::BufferUsageFlagBits::eUniformBuffer,
				VMA_MEMORY_USAGE_AUTO,
				VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
				size, initialData);
			break;
		case BufferType::STORAGE:
			m_pBuffer = m_pCore->createBuffer(vk::BufferUsageFlagBits::eStorageBuffer,
				VMA_MEMORY_USAGE_AUTO, 
				VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
				size, initialData);
			break;
		}
	}

	void Buffer::destroy() {
		m_pCore->destroyBuffer(m_pBuffer);

		m_pBuffer = nullptr;
		m_size = 0;
	}

	void Buffer::resize(size_t newSize)
	{
		void* data = malloc(newSize);
		if (isValid())
		{
			//Save old data
			memcpy(data, m_pBuffer->mappedData, std::min(newSize, getSize()));

			int diff = newSize - getCapacity();
			if (diff > 0) {
				// if space for more initialize it to 0
				memset((char*)data + getCapacity(), 0, diff);
			}
			m_pCore->destroyBuffer(m_pBuffer);
		}
		
		create(m_type, newSize, data);
		free(data);
	}

	size_t Buffer::getCapacity() const {
		return m_pBuffer->size;
	}

	size_t Buffer::getSize() const {
		return m_size;
	}

	BufferType Buffer::getType() const {
		return m_type;
	}

	void sa::Buffer::write(void* data, size_t size, int offset) {
		if (!isValid()) {
			DEBUG_LOG_ERROR("Buffer not initialized! Wrote 0 bytes");
			return;
		}
		if (getCapacity() < size) {
			resize(size);
		}
		memcpy((char*)m_pBuffer->mappedData + offset, data, size);
		m_size = size;
	}

	void Buffer::append(void* data, size_t size) {
		if (!isValid()) {
			DEBUG_LOG_ERROR("Buffer not initialized! Wrote 0 bytes");
			return;
		}
		if (getCapacity() - getSize() < size) {
			resize(getSize() + size);
		}
		
		memcpy((char*)m_pBuffer->mappedData + getSize(), data, size);
		m_size += size;
	}

	void* Buffer::data() {
		return m_pBuffer->mappedData;
	}

	bool Buffer::isValid() const {
		return m_pBuffer != nullptr;
	}
}