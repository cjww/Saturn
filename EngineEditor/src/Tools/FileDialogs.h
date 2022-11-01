#pragma once

#include <filesystem>

namespace sa {
	class FileDialogs {
	public:
		static bool OpenFile(const char* filter, std::filesystem::path& filePath, const std::filesystem::path& initialPath);
		static bool SaveFile(const char* filter, std::filesystem::path& filePath, const std::filesystem::path& initialPath);

	};
}