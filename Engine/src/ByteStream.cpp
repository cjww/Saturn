#pragma once
#include "Tools/ByteStream.h"

namespace sa {
	ByteStream::ByteStream(byte_t* pBytes, size_t size) 
		: m_data(pBytes)
		, m_size(size)
		, m_cursorGet(0)
		, m_cursorPut(0)
	{

	}
	void ByteStream::read(byte_t* pOut, size_t size) {
		assert(m_cursorGet + size <= m_size);
		memcpy(pOut, m_data + m_cursorGet, size);
		m_cursorGet += size;
	}

	void ByteStream::write(byte_t* pIn, size_t size) {
		assert(m_cursorPut + size <= m_size);
		memcpy(m_data + m_cursorPut, pIn, size);
		m_cursorPut += size;
	}

	void ByteStream::seekp(size_t pos) {
		m_cursorPut = pos;
	}

	void ByteStream::seekg(size_t pos) {
		m_cursorGet = pos;
	}

	size_t ByteStream::tellp() const {
		return m_cursorPut;
	}

	size_t ByteStream::tellg() const {
		return m_cursorGet;
	}

}
