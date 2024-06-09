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
		std::vector<ShaderSourceFile> m_sourceFiles;
		
		std::vector<std::vector<uint32_t>> m_code;
		std::vector<Shader> m_shaders;

		PipelineLayout m_depthPipelineLayout;
		PipelineLayout m_colorPipelineLayout;

		ResourceID m_depthPipeline = NULL_RESOURCE;
		ResourceID m_colorPipeline = NULL_RESOURCE;


		bool m_recompiled = false;

	public:
		using Asset::Asset;

		void create(const std::vector<ShaderSourceFile>& sourceFiles);
		void create(const std::vector<std::vector<uint32_t>>& sourceCode);

		void create();

		void recreatePipelines(ResourceID colorRenderProgram, ResourceID depthRenderProgram);
		bool arePipelinesReady() const;

		void bindDepthPipeline(RenderContext& context) const;
		void bindColorPipeline(RenderContext& context) const;

		PipelineLayout& getDepthPipelineLayout();
		PipelineLayout& getColorPipelineLayout();

		const std::vector<ShaderSourceFile>& getShaderSourceFiles() const;
		std::vector<sa::ShaderSourceFile>& getShaderSourceFiles();

		const std::vector<Shader>& getShaders() const;
		bool hasStage(ShaderStageFlagBits stage) const;
		const sa::Shader* getShaderStage(ShaderStageFlagBits stage) const;


		void addShaderSourceFile(const ShaderSourceFile& sourceFile);
		void removeShaderSourceFile(ShaderStageFlagBits stage);

		void compileSource();
		bool isSourceCompiled() const;

		virtual bool onLoad(JsonObject& metaData, AssetLoadFlags flags) override;
		virtual bool onLoadCompiled(ByteStream& dataInStream, AssetLoadFlags flags) override;

		virtual bool onWrite(AssetWriteFlags flags) override;
		virtual bool onCompile(ByteStream& dataOutStream, AssetWriteFlags flags) override;

		virtual bool onUnload() override;

		MaterialShader* clone(const std::string& name, const std::filesystem::path& assetDir = "") const override;



	};
}
