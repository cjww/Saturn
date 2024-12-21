#pragma once
#include <array>
#define SHORT_STRING_LENGTH 128

namespace sa {
	class ShortString {
	private:
		std::array<char, SHORT_STRING_LENGTH> m_buffer;
		void checkAppendLength(const char* str) const;
		void checkLength(const char* str) const;
	public:
		ShortString();
		ShortString(const char* str);

		template<typename ...Args>
		ShortString(const char* format, Args... values);

		const char* data() const;
		uint32_t size() const;

		operator char* ();
		operator const char* ();

		ShortString operator+(const char* right) const;
		ShortString operator+(const std::string& right) const;
	};

	
	template<typename ...Args>
	inline ShortString::ShortString(const char* format, Args ...values) 
		: ShortString()
	{
		std::tuple preArgs = { m_buffer.data(), m_buffer.size(), format };
		std::tuple args = std::tuple_cat(preArgs, std::tuple(values...));
		std::apply(snprintf, args);
	}
}

sa::ShortString operator+(const char* left, const sa::ShortString& right);
