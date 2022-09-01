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
		case sa::BufferType::UNIFORM_TEXEL:
			return "UNIFORM_TEXEL";
		case sa::BufferType::STORAGE_TEXEL:
			return "STORAGE_TEXEL";
		default:
			return "-";
		}
	}

	Buffer::Buffer()
		: m_pBuffer(nullptr)
		, m_pCore(nullptr)
		, m_size(0)
		, m_type(BufferType::VERTEX)
		, m_view(NULL_RESOURCE)
	{

	}

	Buffer::Buffer(VulkanCore* pCore, BufferType type, size_t size, void* initialData) : Buffer() {
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
				VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT,
				size, initialData);
			break;
		case BufferType::INDEX:
			m_pBuffer = m_pCore->createBuffer(vk::BufferUsageFlagBits::eIndexBuffer,
				VMA_MEMORY_USAGE_AUTO, 
				VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT,
				size, initialData);
			break;
		case BufferType::UNIFORM:
			m_pBuffer = m_pCore->createBuffer(vk::BufferUsageFlagBits::eUniformBuffer,
				VMA_MEMORY_USAGE_AUTO,
				VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT,
				size, initialData);
			break;
		case BufferType::STORAGE:
			m_pBuffer = m_pCore->createBuffer(vk::BufferUsageFlagBits::eStorageBuffer,
				VMA_MEMORY_USAGE_AUTO, 
				VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT,
				size, initialData);
			break;
		case BufferType::UNIFORM_TEXEL:
			m_pBuffer = m_pCore->createBuffer(vk::BufferUsageFlagBits::eUniformTexelBuffer,
				VMA_MEMORY_USAGE_AUTO,
				VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT,
				size, initialData);
			m_view = ResourceManager::get().insert<vk::BufferView>(m_pCore->createBufferView(m_pBuffer->buffer, vk::Format::eR32Sfloat));
			break;
		case BufferType::STORAGE_TEXEL:
			m_pBuffer = m_pCore->createBuffer(vk::BufferUsageFlagBits::eStorageTexelBuffer,
				VMA_MEMORY_USAGE_AUTO,
				VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT,
				size, initialData);
			m_view = ResourceManager::get().insert<vk::BufferView>(m_pCore->createBufferView(m_pBuffer->buffer, vk::Format::eR32Sfloat));
			break;
		case BufferType::INDIRECT:
			m_pBuffer = m_pCore->createBuffer(vk::BufferUsageFlagBits::eIndirectBuffer,
				VMA_MEMORY_USAGE_AUTO,
				VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT | VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT,
				size, initialData);
			break;
		}
	}

	void Buffer::destroy() {
		m_pCore->destroyBuffer(m_pBuffer);
		if (m_view != NULL_RESOURCE) {
			ResourceManager::get().remove<vk::BufferView>(m_view);
			m_view = NULL_RESOURCE;
		}
		m_pBuffer = nullptr;
		m_size = 0;
	}

	void Buffer::resize(size_t newSize, BufferResizeFlags resizeFlags) {
		if (!isValid())
			return;
		void* data = nullptr;
		
		if (resizeFlags & BufferResizeFlagBits::PRESERVE_CONTENT) {
			data = malloc(newSize);

			//Save old data
			memcpy(data, m_pBuffer->mappedData, std::min(newSize, getSize()));

			int diff = newSize - getCapacity();
			if (diff > 0) {
				// if space for more initialize it to 0
				memset((char*)data + getCapacity(), 0, diff);
			}
		}
		// Recreate buffer
		m_pCore->destroyBuffer(m_pBuffer);
		create(m_type, newSize, data);
		
		if (data != nullptr)
			free(data);
	}

	void Buffer::reserve(size_t capacity, BufferResizeFlags resizeFlags) {
		if (getCapacity() < capacity) {
			resize(capacity, resizeFlags);
		}
	}


	bool Buffer::setFormat(FormatPrecisionFlags precision, FormatDimensionFlags dimensions, FormatTypeFlags type) {
		if (m_view == NULL_RESOURCE)
			return false;

		vk::Format format = m_pCore->getFormat(precision, dimensions, type, vk::FormatFeatureFlagBits::eUniformTexelBuffer | vk::FormatFeatureFlagBits::eStorageTexelBuffer, vk::ImageTiling::eOptimal);
		if (format != vk::Format::eUndefined) {
			ResourceManager::get().remove<vk::BufferView>(m_view);
			m_view = ResourceManager::get().insert<vk::BufferView>(m_pCore->createBufferView(m_pBuffer->buffer, format));
			return true;
		}
		return false;
	}

	size_t Buffer::getCapacity() const {
		return m_pBuffer->size;
	}

	size_t Buffer::getSize() const {
		return m_size;
	}

	vk::BufferView* Buffer::getView() const {
		return ResourceManager::get().get<vk::BufferView>(m_view);
	}

	BufferType Buffer::getType() const {
		return m_type;
	}

	void sa::Buffer::write(void* data, size_t size, int offset) {
		if (!isValid()) {
			SA_DEBUG_LOG_ERROR("Buffer not initialized! Wrote 0 bytes");
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
			SA_DEBUG_LOG_ERROR("Buffer not initialized! Wrote 0 bytes");
			return;
		}
		if (getCapacity() - getSize() < size) {
			size_t newSize = getSize() + size;
			if (getSize() > 0) newSize = newSize << 1;
			resize(newSize);
		}
		
		memcpy((char*)m_pBuffer->mappedData + getSize(), data, size);
		m_size += size;
	}

	void Buffer::clear() {
		m_size = 0;
	}

	void* Buffer::data() const {
		return m_pBuffer->mappedData;
	}

	bool Buffer::isValid() const {
		return m_pBuffer != nullptr;
	}
}