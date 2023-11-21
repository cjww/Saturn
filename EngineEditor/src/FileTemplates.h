#pragma once
#include <filesystem>
#include <fstream>
#include <Tools\Logger.hpp>
#include <ShaderInfoStructs.h>

namespace sa {

	const char* getTemplateVertexShader();
	const char* getTemplateFragmentShader();
	const char* getTemplateLuaScript();

	void createFile(const std::filesystem::path& path, const char* content);

	void createGlslFile(const std::filesystem::path& path, sa::ShaderStageFlagBits stage);
	void createLuaFile(const std::filesystem::path& path);

}
