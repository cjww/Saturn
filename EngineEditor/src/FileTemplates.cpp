#include "FileTemplates.h"

namespace sa {


	const char* getTemplateVertexShader() {
		return
			"#version 460\n"
			"#extension GL_GOOGLE_include_directive : enable\n"
			"\n"
			"#include \"DefaultRenderPipelineMaterialShaders/DefaultVertexInputs.glsl\"\n"
			"#include \"DefaultRenderPipelineMaterialShaders/DefaultVertexOutputs.glsl\"\n"
			"\n"
			"void main() {\n"
			"	DefaultPassThrough();\n"
			"}";
	}

	const char* getTemplateFragmentShader() {
		return
			"#version 460n\n"
			"#extension GL_GOOGLE_include_directive : enable\n"
			"\n"
			"#include \"DefaultRenderPipelineMaterialShaders/DefaultFragmentInputs.glsl\"\n"
			"#include \"DefaultRenderPipelineMaterialShaders/PBRFunctions.glsl\"\n"
			"\n"
			"layout(location = 0) out vec4 out_color;\n"
			"\n"
			"void main() {\n"
			"	Material material = GetMaterial();\n"
			"	out_color = CalculatePBRColor(material);\n"
			"}";
	}

	const char* getTemplateLuaScript() {
		return
			"Serialize({\n"
			"	-- Place serialized variables here\n"
			"})\n"
			"\n"
			"-- Called once when scene starts running\n"
			"function onStart()\n"
			"\n"
			"end\n"
			"\n"
			"-- Called every frame\n"
			"function onUpdate(deltaTime)\n"
			"\n"
			"end\n";
	}

	void createFile(const std::filesystem::path& path, const char* content) {
		std::ofstream file(path);
		if (!file.good()) {
			SA_DEBUG_LOG_ERROR("Failed to open ", path);
		}

		file << content;

		file.close();
	}

	void createGlslFile(const std::filesystem::path& path, sa::ShaderStageFlagBits stage) {
		
		switch (stage)
		{
		case sa::VERTEX:
			createFile(path, getTemplateVertexShader());
			break;
		case sa::TESSELLATION_CONTROL:
			break;
		case sa::TESSELLATION_EVALUATION:
			break;
		case sa::GEOMETRY:
			break;
		case sa::FRAGMENT:
			createFile(path, getTemplateFragmentShader());
			break;
		case sa::COMPUTE:
			break;
		default:
			break;
		}
	}

	void createLuaFile(const std::filesystem::path& path) {
		createFile(path, getTemplateLuaScript());
	}
}