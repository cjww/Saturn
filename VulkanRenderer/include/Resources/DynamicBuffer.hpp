#pragma once

#include "Resources\Buffer.hpp"

namespace sa {
	// Dynamic buffers can expand mid frame
	class DynamicBuffer {
	private:
		std::vector<Buffer> m_buffers;
		uint32_t m_currentBufferIndex;

		friend class RenderContext;
		void setBufferIndex(uint32_t index);
	public:

		DynamicBuffer();
		DynamicBuffer(BufferType type, size_t size = 0, void* initialData = nullptr);

		DynamicBuffer(const DynamicBuffer&) = delete;
		DynamicBuffer(DynamicBuffer&&) = default;
		DynamicBuffer& operator=(const DynamicBuffer&) = default;

		void create(BufferType type, size_t size = 0, void* initialData = nullptr);
		void destroy();

		//bool setFormat(FormatPrecisionFlags precision, FormatDimensionFlags dimensions, FormatTypeFlags type);

		const sa::Buffer& getBuffer() const;
		sa::Buffer& getBuffer(uint32_t index);

		
		uint32_t getBufferIndex() const;
		uint32_t getPreviousBufferIndex() const;
		uint32_t getNextBufferIndex() const;
		
		void swap();

		uint32_t getBufferCount() const;

		BufferType getType() const;

		void write(void* data, size_t size, int offset = 0);
		void append(void* data, size_t size, int alignment = 0);

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

		void copy(const Buffer& other);

		void resize(size_t newSize, BufferResizeFlags resizeFlags = PRESERVE_CONTENT);
		void reserve(size_t capacity, BufferResizeFlags resizeFlags = PRESERVE_CONTENT);

		void* data();
		void* data(uint32_t offset);

		void clear();

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

		operator const Buffer& ();
		
		template<typename T>
		T& at(uint32_t index) const;

	};

	template<typename T>
	inline void DynamicBuffer::write(const T& data) {
		write((void*)&data, sizeof(T));
	}

	template<typename T>
	inline void DynamicBuffer::write(const std::vector<T>& data) {
		write((void*)data.data(), data.size() * sizeof(T));
	}

	template<typename T, size_t Size>
	inline void DynamicBuffer::write(const std::array<T, Size>& data) {
		write((void*)data.data(), data.size() * sizeof(T));
	}
	
	template<typename T>
	inline void DynamicBuffer::append(const T& data, int alignment) {
		append((void*)&data, sizeof(T), alignment);
	}

	template<typename T>
	inline void DynamicBuffer::append(const std::vector<T>& data, int alignment) {
		append((void*)data.data(), data.size() * sizeof(T), alignment);
	}

	template<typename T, size_t Size>
	inline void DynamicBuffer::append(const std::array<T, Size>& data, int alignment) {
		append((void*)data.data(), Size * sizeof(T), alignment);
	}

	template<typename T>
	inline size_t DynamicBuffer::getElementCount() const{
		return m_buffers[m_currentBufferIndex].getElementCount<T>();
	}

	template<typename T>
	inline std::vector<T> DynamicBuffer::getContent() {
		return m_buffers[m_currentBufferIndex].getContent<T>();
	}

	template<typename T>
	inline Buffer& DynamicBuffer::operator<<(const T& value) {
		return m_buffers[m_currentBufferIndex] << value;
	}

	template<typename T>
	inline Buffer& DynamicBuffer::operator<<(std::vector<T>& values) {
		return m_buffers[m_currentBufferIndex] << values;
	}

	template<typename T, size_t Size>
	inline Buffer& DynamicBuffer::operator<<(std::array<T, Size>& values) {
		return m_buffers[m_currentBufferIndex] << values;
	}

	template<typename T>
	inline T& DynamicBuffer::at(uint32_t index) const {
		return m_buffers[m_currentBufferIndex].at<T>(index);
	}

}