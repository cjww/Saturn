#pragma once

#include <filesystem>

namespace sa {
	class FileDialogs {
	public:
		static bool OpenFile(const char* filter, std::filesystem::path& filePath, const std::filesystem::path& initialPath = std::filesystem::current_path());
		static bool SaveFile(const char* filter, std::filesystem::path& filePath, const std::filesystem::path& initialPath = std::filesystem::current_path());

	};
}