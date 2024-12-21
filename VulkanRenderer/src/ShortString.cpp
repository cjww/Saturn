#include "pch.h"
#include "Tools/ShortString.hpp"

namespace sa {
	void ShortString::checkAppendLength(const char* str) const {
		size_t currentLen = strlen(m_buffer.data());
		size_t spaceLeft = m_buffer.size() - currentLen - 1;
		if (strlen(str) > spaceLeft)
			throw std::runtime_error(ShortString("String to long! Len: %lu, Str: %s", strlen(str), str));
	}

	void ShortString::checkLength(const char* str) const {
		if (strlen(str) > m_buffer.size() - 1)
			throw std::runtime_error(ShortString("String to long! Len: %lu, Str: %s", strlen(str), str));
	}

	ShortString::ShortString()
		: m_buffer()
	{
		memset(m_buffer.data(), '\0', m_buffer.size());
	};
	
	ShortString::ShortString(const char* str) 
		: ShortString()
	{
		checkLength(str);
		snprintf(m_buffer.data(), m_buffer.size(), str);
	}

	const char* ShortString::data() const {
		return m_buffer.data();
	}

	ShortString::operator char* () {
		return m_buffer.data();
	}

	uint32_t ShortString::size() const {
		return strlen(data());
	}

	ShortString::operator const char* () {
		return m_buffer.data();
	}

	ShortString ShortString::operator+(const char* right) const {
		checkAppendLength(right);
		ShortString str = *this;
		int len = strlen(str);
		snprintf(str.m_buffer.data() + len, str.m_buffer.size() - len, right);
		return str;
	}

	ShortString ShortString::operator+(const std::string& right) const {
		return *this + right.c_str();
	}
}

sa::ShortString operator+(const char* left, const sa::ShortString& right) {
	return sa::ShortString(left) + right;
}
