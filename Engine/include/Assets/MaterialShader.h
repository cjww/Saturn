#pragma once
#include "Asset.h"
#include <ShaderSet.hpp>
#include <structs.h>
#include <Renderer.hpp>

namespace sa {

	struct ShaderSourceFile {
		std::filesystem::path filePath;
		ShaderStageFlagBits stage;
	};

	class MaterialShader : public Asset {
	private:
		friend class MaterialShaderCollection;

		std::vector<ShaderSourceFile> m_sourceFiles;
		
		std::vector<std::vector<uint32_t>> m_code;
		std::vector<Shader> m_shaders;
		PipelineLayout m_colorPipelineLayout;
		PipelineLayout m_depthPipelineLayout;


		bool m_recompiled = false;

	public:
		using Asset::Asset;

		void create(const std::vector<ShaderSourceFile>& sourceFiles);
		void create(const std::vector<std::vector<uint32_t>>& sourceCode);

		void create();

		
		const std::vector<ShaderSourceFile>& getShaderSourceFiles() const;
		std::vector<sa::ShaderSourceFile>& getShaderSourceFiles();

		void addShaderSourceFile(const ShaderSourceFile& sourceFile);
		void removeShaderSourceFile(ShaderStageFlagBits stage);

		void compileSource();
		bool isCompiled() const;

		virtual bool onLoad(std::ifstream& file, AssetLoadFlags flags) override;
		virtual bool onWrite(std::ofstream& file, AssetWriteFlags flags) override;
		virtual bool onUnload() override;


	};
}