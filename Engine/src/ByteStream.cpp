#pragma once
#include "Tools/ByteStream.h"

namespace sa {
	ByteStream::ByteStream(byte_t* pBytes, size_t size)
		: m_data(pBytes)
		, m_size(size)
		, m_capacity(size)
		, m_cursorGet(0)
		, m_cursorPut(0)
		, m_owningData(false)
	{

	}

	ByteStream::ByteStream(size_t initialCapacity)
		: m_data(new byte_t[initialCapacity])
		, m_size(0)
		, m_capacity(initialCapacity)
		, m_cursorGet(0)
		, m_cursorPut(0)
		, m_owningData(true)
	{

	}

	ByteStream::~ByteStream() {
		if (m_owningData) {
			delete m_data;
		}
	}
	
	void ByteStream::read(byte_t* pOut, size_t size) {
		assert(m_cursorGet + size <= m_capacity);
		memcpy(pOut, m_data + m_cursorGet, size);
		m_cursorGet += size;
	}

	void ByteStream::write(const byte_t* pIn, size_t size) {
		if (m_cursorPut + size > m_capacity) {
			if (!m_owningData) {
				throw std::runtime_error("Buffer too small");
			}

			// if owning data, then safe to resize
			size_t newSize = m_capacity;
			while (m_cursorPut + size > newSize) {
				newSize = newSize << 1; // double the size
			}
			byte_t* tmp = new byte_t[newSize];
			memcpy(tmp, m_data, m_size);
			m_capacity = newSize;
			delete m_data;
			m_data = tmp;
		}
		memcpy(m_data + m_cursorPut, pIn, size);
		m_cursorPut += size;
		m_size = std::max(m_cursorPut, m_size);
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

	byte_t* ByteStream::data() const {
		return m_data;
	}

	size_t ByteStream::size() const {
		return m_size;
	}

}
