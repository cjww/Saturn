#pragma once

#include "IAsset.h"
#include "Graphics/Material.h"
namespace sa {
	class MaterialAsset : public IAsset {
	private:
		friend class AssetManager;
		inline static AssetTypeID s_typeID;
	public:
		using IAsset::IAsset;
		static AssetTypeID type() { return s_typeID; }

		Material data;

		virtual bool onLoad(std::ifstream& file, AssetLoadFlags flags) override;
		virtual bool onWrite(std::ofstream& file, AssetWriteFlags flags) override;
		virtual bool onUnload() override;

	};
}