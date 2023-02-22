#pragma once

#include "IAsset.h"
#include "Graphics/Material.h"
namespace sa {
	class MaterialAsset : public IAsset {
	public:
		using IAsset::IAsset;
		
		Material data;

		virtual bool onLoad(std::ifstream& file, AssetLoadFlags flags) override;
		virtual bool onWrite(std::ofstream& file, AssetWriteFlags flags) override;
		virtual bool onUnload() override;

	};
}