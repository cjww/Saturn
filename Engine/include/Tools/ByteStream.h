#pragma once
#include <cstdint>
#include <sstream>
namespace sa {

	using byte_t = uint8_t;

	class ByteStream {
	private:
		size_t m_size;
		size_t m_cursorPut;
		size_t m_cursorGet;

		byte_t* m_data;

	public:
		ByteStream(byte_t* pBytes, size_t size);

		void read(byte_t* pOut, size_t size);
		void write(byte_t* pIn, size_t size);

		void seekp(size_t pos);
		void seekg(size_t pos);

		size_t tellp() const;
		size_t tellg() const;

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
			write(static_cast<byte_t*>(&pIn), sizeof(T));
		}
		else {
			write(reinterpret_cast<byte_t*>(&pIn), sizeof(T));
		}
	}

}