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

		Extent m_currentExtent;

		ShaderSet m_colorShaderSet;
		ShaderSet m_depthShaderSet;

		ResourceID m_colorPipeline = NULL_RESOURCE;
		ResourceID m_depthPipeline = NULL_RESOURCE;

	public:
		using IAsset::IAsset;

		void create(const std::vector<ShaderSourceFile>& sourceFiles);
		void create(const std::vector<std::vector<uint32_t>>& code);

		void recreatePipelines(ResourceID colorRenderProgram, ResourceID depthRenderProgram, Extent extent);

		void bindColorPipeline(RenderContext& context);
		void bindDepthPipeline(RenderContext& context);

		virtual bool onLoad(std::ifstream& file, AssetLoadFlags flags) override;
		virtual bool onWrite(std::ofstream& file, AssetWriteFlags flags) override;
		virtual bool onUnload() override;


	};
}
