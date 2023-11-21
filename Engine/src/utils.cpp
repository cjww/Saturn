#include "pch.h"
#include "Tools/utils.h"

namespace sa {
	namespace utils {

		char toLower(char c) {
			if (c >= 'A' && c <= 'Z')
				return c + 32;
			return c;
		}
		char toUpper(char c) {
			if (c >= 'a' && c <= 'z')
				return c - 32;
			return c;
		}

		std::string toUpper(const std::string& str) {
			std::string newStr;
			for (const char& c : str) {
				char nc = toUpper(c);
				newStr.push_back(nc);
			}
			return newStr;
		}

		std::string toLower(const std::string& str) {
			std::string newStr;
			for (const char& c : str) {
				char nc = toLower(c);
				newStr.push_back(nc);
			}
			return newStr;
		}

		std::string trim(const std::string& str) {
			std::string newStr;
			int i = 0;
			while (str[i] == '\t' || str[i] == ' ') {
				i++;
			}
			newStr = str.substr(i);
			i = newStr.size() - 1;
			while (newStr[i] == '\t' || newStr[i] == ' ') {
				i--;
			}
			newStr = newStr.substr(0, i + 1);
			return newStr;
		}

		std::vector<std::string> split(const std::string& str, char seperator, bool trimWhiteSpace) {
			std::vector<std::string> substrings;

			std::string buffer;
			for (char c : str) {
				if (c == seperator) {
					if (trimWhiteSpace) {
						buffer = trim(buffer);
					}
					substrings.push_back(buffer);
					buffer.clear();
				}
				else {
					buffer.push_back(c);
				}
			}
			if (buffer.size() != 0) {
				substrings.push_back(buffer);
			}

			return substrings;

		}

		void stripTypeName(std::string & str) {
			auto p = str.find_last_of("::");
			if (p != std::string::npos) {
				str = str.substr(p + 1);
			}
			str = str.substr(0, str.find_first_of("> "));
		}
		
	}
}
