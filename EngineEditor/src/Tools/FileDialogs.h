#pragma once

#include <filesystem>

namespace sa {
	class FileDialogs {
	public:
		static bool OpenFile(const char* filter, std::filesystem::path& filePath, const std::filesystem::path& initialPath = std::filesystem::current_path());
		static bool SaveFile(const char* filter, std::filesystem::path& filePath, const std::filesystem::path& initialPath = std::filesystem::current_path());

		static bool OpenFileInTextEditor(const std::filesystem::path& applicationPath, const std::filesystem::path& file);
		
		// Modal yes or no selection window, returns true if yes was pressed and false otherwise
		static bool YesNoWindow(const wchar_t* pTitle, const wchar_t* pMsg, bool isYesDefault = false);
	};
}