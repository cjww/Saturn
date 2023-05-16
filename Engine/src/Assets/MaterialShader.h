#pragma once
#include "IAsset.h"
#include <ShaderSet.hpp>
#include <structs.h>
#include <Renderer.hpp>

namespace sa {
	class MaterialShader {
	private:
		friend class MaterialShaderCollection;

		Extent m_currentExtent;

		ShaderSet m_colorShaderSet;
		ShaderSet m_depthShaderSet;

		ResourceID m_colorPipeline = NULL_RESOURCE;
		ResourceID m_depthPipeline = NULL_RESOURCE;

	public:
		//using IAsset::IAsset;

		MaterialShader();

		void create(const std::vector<std::vector<uint32_t>>& code);

		void recreatePipelines(ResourceID colorRenderProgram, ResourceID depthRenderProgram, Extent extent);

		void bindColorPipeline(RenderContext& context);
		void bindDepthPipeline(RenderContext& context);

		/*
		virtual bool onLoad(std::ifstream& file, AssetLoadFlags flags) override;
		virtual bool onWrite(std::ofstream& file, AssetWriteFlags flags) override;
		virtual bool onUnload() override;

		*/
		

	};
}
