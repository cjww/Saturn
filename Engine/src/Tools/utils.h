#pragma once

namespace sa {
	namespace utils {
		char toLower(char c);
		char toUpper(char c);

		std::string toUpper(const std::string& str);
		std::string toLower(const std::string& str);

		std::string trim(const std::string& str);

		std::vector<std::string> split(const std::string& str, char seperator, bool trimWhiteSpace = true);
	}

}