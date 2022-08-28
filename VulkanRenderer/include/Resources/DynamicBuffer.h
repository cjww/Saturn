#pragma once

#include "Resources\Buffer.hpp"

namespace sa {
	class DynamicBuffer {
	private:
		std::vector<Buffer> m_buffers;
		uint32_t m_currentBufferIndex;

		friend class RenderContext;
		void setBufferIndex(uint32_t index);
	public:

		DynamicBuffer();
		DynamicBuffer(VulkanCore* pCore, BufferType type, uint32_t bufferCount, size_t size, void* initialData);

		void destroy();

		//bool setFormat(FormatPrecisionFlags precision, FormatDimensionFlags dimensions, FormatTypeFlags type);

		sa::Buffer getBuffer(uint32_t index = -1) const;
		uint32_t getBufferCount() const;

		BufferType getType() const;

		void write(void* data, size_t size, int offset = 0);
		void append(void* data, size_t size);

		template<typename T>
		void write(const T& data);

		template<typename T>
		void write(const std::vector<T>& data);

		template<typename T, size_t Size>
		void write(const std::array<T, Size>& data);

		void* data();

		template<typename T>
		size_t getElementCount() const;

		template<typename T>
		std::vector<T> getContent();

		bool isValid() const;

		template<typename T>
		Buffer& operator<<(const T& value);

		template<typename T>
		Buffer& operator<<(std::vector<T>& values);

		template<typename T, size_t Size>
		Buffer& operator<<(std::array<T, Size>& values);

	};

	template<typename T>
	inline void DynamicBuffer::write(const T& data) {
		write(&data, sizeof(T));
	}

	template<typename T>
	inline void DynamicBuffer::write(const std::vector<T>& data) {
		write(data.data(), data.size() * sizeof(T));
	}

	template<typename T, size_t Size>
	inline void DynamicBuffer::write(const std::array<T, Size>& data) {
		write(data.data(), data.size() * sizeof(T));
	}
	
	template<typename T>
	inline size_t DynamicBuffer::getElementCount() const{
		return getBuffer().getElementCount<T>();
	}

	template<typename T>
	inline std::vector<T> DynamicBuffer::getContent() {
		return getBuffer().getContent<T>();
	}

	template<typename T>
	inline Buffer& DynamicBuffer::operator<<(const T& value) {
		return getBuffer() << value;
	}

	template<typename T>
	inline Buffer& DynamicBuffer::operator<<(std::vector<T>& values) {
		return getBuffer() << values;
	}

	template<typename T, size_t Size>
	inline Buffer& DynamicBuffer::operator<<(std::array<T, Size>& values) {
		return getBuffer() << values;
	}
}