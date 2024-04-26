#pragma once
#include <cstdint>
#include <sstream>
namespace sa {

	using byte_t = uint8_t;

	class ByteStream {
	private:
		size_t m_size;
		size_t m_capacity;
		size_t m_cursorPut;
		size_t m_cursorGet;

		byte_t* m_data;

		const bool m_owningData;

	public:
		ByteStream(byte_t* pBytes, size_t size); // reading
		ByteStream(size_t initialSize); // writing
		~ByteStream();

		void read(byte_t* pOut, size_t size);
		void write(const byte_t* pIn, size_t size);

		void seekp(size_t pos);
		void seekg(size_t pos);

		size_t tellp() const;
		size_t tellg() const;

		byte_t* data() const;
		size_t size() const;


		template<typename T>
		void read(T* pOut);

		template<typename T>
		void write(const T& pIn);


	};

	template<typename T>
	inline void ByteStream::read(T* pOut) {
		read(reinterpret_cast<byte_t*>(pOut), sizeof(T));
	}

	template<typename T>
	inline void ByteStream::write(const T& pIn) {
		if constexpr (std::is_nothrow_convertible<T*, byte_t*>::value) {
			write(static_cast<const byte_t*>(&pIn), sizeof(T));
		}
		else {
			write(reinterpret_cast<const byte_t*>(&pIn), sizeof(T));
		}
	}

}