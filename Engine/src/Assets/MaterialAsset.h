#pragma once

#include "IAsset.h"
#include "Graphics/Material.h"
namespace sa {
	class MaterialAsset : public IAsset {
	private:
		friend class AssetManager;
		inline static AssetTypeID s_typeID;
		virtual bool unload() override;
	public:
		using IAsset::IAsset;
		static AssetTypeID type() { return s_typeID; }

		Material data;

		virtual bool create(const std::string& name) override;

		virtual bool load() override;
		virtual bool write() override;

	};
}