#pragma once
#include "IAsset.h"
#include <ShaderSet.hpp>

namespace sa {
	class MaterialShader : public IAsset {
	private:
		ShaderSet m_shaderSet;
		ResourceID m_pipeline;
	public:
		using IAsset::IAsset;
		
		virtual bool onLoad(std::ifstream& file, AssetLoadFlags flags) override;
		virtual bool onWrite(std::ofstream& file, AssetWriteFlags flags) override;
		virtual bool onUnload() override;

	};
}
