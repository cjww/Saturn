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
	class Buffer {
	private:
		VulkanCore* m_pCore;
		DeviceBuffer* m_pBuffer;
		size_t m_elementCount;

		BufferType m_type;

	public:

		Buffer();
		Buffer(VulkanCore* pCore, BufferType type, size_t size, size_t elementCount, void* initialData);

		void create(BufferType type, size_t size, void* initialData);
		void destroy();

		void resize(size_t newSize);

		size_t getElementCount() const;
		size_t getSize() const;

		void write(void* data, size_t size, size_t elementCount, int offset = 0);

		template<typename T>
		void write(const T& data);

		template<typename T>
		void write(std::vector<T>& data);

		void* data();

		template<typename T>
		auto getContent();

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
	inline void Buffer::write(const T& data) {
		write((void*)&data, sizeof(T), 1);
	}

	template<typename T>
	inline void Buffer::write(std::vector<T>& data) {
		write((void*)data.data(), data.size() * sizeof(T), data.size());
	}

	template<typename T>
	inline auto Buffer::getContent() {
		std::vector<T> elements(getElementCount());
		memcpy(elements.data(), data(), getSize());
		return elements;
	}
	
	template<typename T>
	inline Buffer& Buffer::operator<<(const T& value) {
		write(value);
		return *this;
	}
}