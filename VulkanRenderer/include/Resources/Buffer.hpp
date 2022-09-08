#pragma once

#include "FormatFlags.hpp"

#include "ResourceManager.hpp"

namespace vk {
	class BufferView;
}

namespace sa {
	struct DeviceBuffer;
	class VulkanCore;

	enum class BufferType {
		VERTEX,
		INDEX,
		UNIFORM,
		STORAGE,
		UNIFORM_TEXEL,
		STORAGE_TEXEL,
		INDIRECT,
	};

	std::string to_string(const BufferType& value);

	typedef uint32_t BufferResizeFlags;

	enum BufferResizeFlagBits : BufferResizeFlags {
		IGNORE_CONTENT = 0,
		PRESERVE_CONTENT = 1
	};

	class Buffer {
	private:
		VulkanCore* m_pCore;
		DeviceBuffer* m_pBuffer;
		size_t m_size;

		ResourceID m_view;

		BufferType m_type;

	public:

		Buffer();
		Buffer(VulkanCore* pCore, BufferType type, size_t size, void* initialData);

		void create(BufferType type, size_t size, void* initialData);
		void destroy();

		// Resizes the buffer
		void resize(size_t newSize, BufferResizeFlags resizeFlags = PRESERVE_CONTENT);
		// Only expands the buffer if new capacity is bigger than current capacity, otherwise does nothing
		void reserve(size_t capacity, BufferResizeFlags resizeFlags = PRESERVE_CONTENT);

		bool setFormat(FormatPrecisionFlags precision, FormatDimensionFlags dimensions, FormatTypeFlags type);

		size_t getCapacity() const;
		size_t getSize() const;

		vk::BufferView* getView() const;

		BufferType getType() const;

		void write(void* data, size_t size, int offset = 0);
		void append(void* data, size_t size, int alignment = 0);
		
		void clear();

		template<typename T>
		size_t getElementCount() const;

		template<typename T>
		void write(const T& data);

		template<typename T>
		void write(const std::vector<T>& data);
		
		template<typename T, size_t Size>
		void write(const std::array<T, Size>& data);

		template<typename T>
		void append(const T& data, int alignment = 0);

		template<typename T>
		void append(const std::vector<T>& data, int alignment = 0);

		template<typename T, size_t Size>
		void append(const std::array<T, Size>& data, int alignment = 0);
		


		void* data() const;

		template<typename T>
		std::vector<T> getContent() const;

		bool isValid() const;

		operator const DeviceBuffer*() const {
			return m_pBuffer;
		}

		template<typename T>
		Buffer& operator<<(const T& value);

		template<typename T>
		Buffer& operator<<(const std::vector<T>& values);

		template<typename T, size_t Size>
		Buffer& operator<<(const std::array<T, Size>& values);

		template<typename T>
		T& at(uint32_t index) const;

	};

	template<typename T>
	inline size_t Buffer::getElementCount() const {
		return getSize() / sizeof(T);
	}


	template<typename T>
	inline void Buffer::write(const T& data) {
		write((void*)&data, sizeof(T));
	}

	template<typename T>
	inline void Buffer::write(const std::vector<T>& data) {
		write((void*)data.data(), data.size() * sizeof(T));
	}

	template<typename T, size_t Size>
	inline void Buffer::write(const std::array<T, Size>& data) {
		write((void*)data.data(), Size * sizeof(T));
	}

	template<typename T>
	inline void Buffer::append(const T& data, int alignment) {
		append((void*)&data, sizeof(T), alignment);
	}

	template<typename T>
	inline void Buffer::append(const std::vector<T>& data, int alignment) {
		append((void*)data.data(), data.size() * sizeof(T), alignment);
	}

	template<typename T, size_t Size>
	inline void Buffer::append(const std::array<T, Size>& data, int alignment) {
		append((void*)data.data(), Size * sizeof(T), alignment);
	}

	template<typename T>
	inline std::vector<T> Buffer::getContent() const {
		std::vector<T> elements(getElementCount<T>());
		memcpy(elements.data(), data(), getSize());
		return std::move(elements);
	}
	
	template<typename T>
	inline Buffer& Buffer::operator<<(const T& value) {
		append((void*)&value, sizeof(T));
		return *this;
	}

	template<typename T>
	inline Buffer& Buffer::operator<<(const std::vector<T>& values) {
		append((void*)values.data(), values.size() * sizeof(T));
		return *this;
	}

	template<typename T, size_t Size>
	inline Buffer& Buffer::operator<<(const std::array<T, Size>& values) {
		append((void*)values.data(), Size * sizeof(T));
		return *this;
	}

	template<typename T>
	T& Buffer::at(uint32_t index) const {
		T* arr = (T*)data();
		return arr[index];
	}
}