#pragma once

namespace sa {
	struct DeviceBuffer;
	class VulkanCore;

	enum class BufferType {
		VERTEX,
		INDEX,
		UNIFORM,
		STORAGE
	};

	std::string to_string(const BufferType& value);

	class Buffer {
	private:
		VulkanCore* m_pCore;
		DeviceBuffer* m_pBuffer;
		size_t m_size;

		BufferType m_type;

	public:

		Buffer();
		Buffer(VulkanCore* pCore, BufferType type, size_t size, void* initialData);

		void create(BufferType type, size_t size, void* initialData);
		void destroy();

		void resize(size_t newSize);

		size_t getCapacity() const;
		size_t getSize() const;

		BufferType getType() const;

		void write(void* data, size_t size, int offset = 0);
		void append(void* data, size_t size);

		template<typename T>
		size_t getElementCount() const;

		template<typename T>
		void write(const T& data);

		template<typename T>
		void write(const std::vector<T>& data);
		
		template<typename T, size_t Size>
		void write(const std::array<T, Size>& data);

		void* data();

		template<typename T>
		std::vector<T> getContent();

		bool isValid() const;

		operator const DeviceBuffer*() const {
			return m_pBuffer;
		}

		operator DeviceBuffer*() {
			return m_pBuffer;
		}

		template<typename T>
		Buffer& operator<<(const T& value);

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
	inline std::vector<T> Buffer::getContent() {
		std::vector<T> elements(getElementCount<T>());
		memcpy(elements.data(), data(), getSize());
		return elements;
	}
	
	template<typename T>
	inline Buffer& Buffer::operator<<(const T& value) {
		append((void*)&value, sizeof(T));
		return *this;
	}
}