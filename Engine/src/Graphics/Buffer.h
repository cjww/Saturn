#pragma once

namespace vr {
	struct Buffer;
}

namespace sa {
	enum class BufferType {
		VERTEX,
		INDEX,
		UNIFORM,
		STORAGE
	};
	class Buffer {
	private:
		vr::Buffer* m_pBuffer = nullptr;
		size_t m_elementCount;

		BufferType m_type;

		void init(size_t size, void* initialData);

	public:

		Buffer() = default;
		Buffer(BufferType type, size_t size = 0, size_t elementCount = 0, void* initialData = nullptr);

		void resize(size_t newSize);

		size_t getElementCount() const;
		size_t getSize() const;

		void write(void* data, size_t size, size_t elementCount, int offset = 0);

		template<typename T>
		void write(const T& data);

		template<typename T>
		void write(std::vector<T>& data);

		void* data();

		bool isValid() const;

		operator const vr::Buffer*() const {
			return m_pBuffer;
		}

		operator vr::Buffer* () {
			return m_pBuffer;
		}

	};

	template<typename T>
	inline void Buffer::write(const T& data) {
		write((void*)&data, sizeof(T), 1);
	}

	template<typename T>
	inline void Buffer::write(std::vector<T>& data) {
		write((void*)data.data(), data.size() * sizeof(T), data.size());
	}
}