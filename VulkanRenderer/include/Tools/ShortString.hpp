#pragma once
#include <array>
#include <string>

constexpr size_t SHORT_STRING_LENGTH = 128;
namespace sa {
	class ShortString {
	private:
		std::array<char, SHORT_STRING_LENGTH> m_buffer;
		bool canAppendLength(size_t len) const;
		bool canAssignLength(size_t len) const;
	public:
		static constexpr uint32_t MaxStringLength = SHORT_STRING_LENGTH - 1;

		ShortString();
		ShortString(const char* str);
		ShortString(const std::string_view& view);

		template<typename ...Args>
		ShortString(const char* format, Args... values);
 
		bool tryAssign(const char* str);
		bool tryAssign(const std::string_view& view);

		template<typename ...Args>
		bool tryAssign(const char* format, Args... values);

		int getMaxAppendLength() const;

		const char* data() const;
		uint32_t size() const;
		uint32_t length() const;

		std::array<char, SHORT_STRING_LENGTH>::iterator begin();
		std::array<char, SHORT_STRING_LENGTH>::iterator end();

		operator char* ();
		operator const char* ();

		ShortString operator+(const char* right) const;
		ShortString operator+(const std::string& right) const;
	};

	
	template<typename ...Args>
	inline ShortString::ShortString(const char* format, Args ...values) 
		: ShortString()
	{
		if(!tryAssign(format, values...))
			throw std::runtime_error(ShortString("Formated string to long!"));
	}
    
	template <typename... Args>
    inline bool ShortString::tryAssign(const char *format, Args... values) {
		try {
			std::tuple preArgs = { m_buffer.data(), m_buffer.size(), format };
			std::tuple args = std::tuple_cat(preArgs, std::tuple(values...));
			std::apply(snprintf, args);
		}
		catch(const std::exception& e) {
			return false;
		}
        return true;
    }
}

sa::ShortString operator+(const char* left, const sa::ShortString& right);
