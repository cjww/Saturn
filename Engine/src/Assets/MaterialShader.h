#pragma once
#include "IAsset.h"
#include <ShaderSet.hpp>
#include <structs.h>
#include <Renderer.hpp>

namespace sa {

	struct ShaderSourceFile {
		std::filesystem::path filePath;
		ShaderStageFlagBits stage;
	};

	class MaterialShader : public IAsset {
	private:
		friend class MaterialShaderCollection;

		std::vector<ShaderSourceFile> m_sourceFiles;

		std::vector<std::vector<uint32_t>> m_code;

		ShaderSet m_colorShaderSet;
		ShaderSet m_depthShaderSet;

		bool m_recompiled = false;

	public:
		using IAsset::IAsset;

		void create(const std::vector<ShaderSourceFile>& sourceFiles);
		void create(const std::vector<std::vector<uint32_t>>& code);

		
		const std::vector<ShaderSourceFile>& getShaderSourceFiles() const;
		std::vector<sa::ShaderSourceFile>& getShaderSourceFiles();

		void addShaderSourceFile(const ShaderSourceFile& sourceFile);
		void removeShaderSourceFile(ShaderStageFlagBits stage);

		void compileSource();

		virtual bool onLoad(std::ifstream& file, AssetLoadFlags flags) override;
		virtual bool onWrite(std::ofstream& file, AssetWriteFlags flags) override;
		virtual bool onUnload() override;


	};
}
