#pragma once
#include "IAsset.h"
#include <ShaderSet.hpp>

namespace sa {
	class MaterialShader : public IAsset {
	private:
		friend class MaterialShaderCollection;

		ShaderSet m_colorShaderSet;
		ShaderSet m_depthShaderSet;
		ResourceID m_colorPipeline;
		ResourceID m_depthPipeline;

	public:
		using IAsset::IAsset;
		
		virtual bool onLoad(std::ifstream& file, AssetLoadFlags flags) override;
		virtual bool onWrite(std::ofstream& file, AssetWriteFlags flags) override;
		virtual bool onUnload() override;

	};
}
