#include "pch.h"
#include "Tools/ShortString.hpp"

namespace sa {
	bool ShortString::canAppendLength(size_t len) const {
		size_t currentLen = length();
		size_t spaceLeft = MaxStringLength - currentLen;
		if (len > spaceLeft)
			throw std::runtime_error(ShortString("String to long! Len: %lu", len));
	}

	bool ShortString::canAssignLength(size_t len) const {
		if (len > MaxStringLength)
			return false;
		return true;
	}

    ShortString::ShortString()
        : m_buffer()
    {
		memset(m_buffer.data(), '\0', m_buffer.size());
	};
	
	ShortString::ShortString(const char* str) 
		: ShortString()
	{
		if(!tryAssign(str))
			throw std::runtime_error(ShortString("String to long! Length: %lu, String: %s", strlen(str), str));
	}

	ShortString::ShortString(const std::string_view& view)
		: ShortString()
	{
		if(!tryAssign(view))
			throw std::runtime_error(ShortString("String to long! Length: %lu, String: %s", view.length(), view.data()));
	}

    bool ShortString::tryAssign(const char *str) {
		int len = strlen(str);
		if(!canAssignLength(len))
			return false;
		snprintf(m_buffer.data(), MaxStringLength, str);
        return true;
    }

	bool ShortString::tryAssign(const std::string_view& view) {
		if (!canAssignLength(view.length()))
			return false;
		snprintf(m_buffer.data(), MaxStringLength, view.data());
		return true;
	}

    int ShortString::getMaxAppendLength() const {
        size_t currentLen = length();
		return MaxStringLength - currentLen;;
    }

    const char *ShortString::data() const {
	    return m_buffer.data();
    }

    uint32_t ShortString::size() const {
		return strlen(data());
	}
	
    uint32_t ShortString::length() const {
        return size();
    }

	std::array<char, SHORT_STRING_LENGTH>::iterator ShortString::begin() {
		return m_buffer.begin();
	}

    std::array<char, SHORT_STRING_LENGTH>::iterator ShortString::end()
    {
        return m_buffer.end() - 1;
    }

    ShortString::operator char* () {
		return m_buffer.data();
	}

	ShortString::operator const char* () {
		return m_buffer.data();
	}

	ShortString ShortString::operator+(const char* right) const {
		int len = strlen(right);
		if(!canAppendLength(len))
			throw std::runtime_error(ShortString("String to long! Length: %lu, String: %s", len, right));
		ShortString str = *this;
		int currentLen = str.length();
		snprintf(str.m_buffer.data() + currentLen, str.m_buffer.size() - currentLen, right);
		return str;
	}

	ShortString ShortString::operator+(const std::string& right) const {
		return *this + right.c_str();
	}
}

sa::ShortString operator+(const char* left, const sa::ShortString& right) {
	return sa::ShortString(left) + right;
}
