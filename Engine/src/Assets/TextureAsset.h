#pragma once

#include "IAsset.h"
#include "Resources/Texture.hpp"

namespace sa {
	class TextureAsset : public IAsset {
	private:
		friend class AssetManager;
		inline static AssetTypeID s_typeID;

		std::vector<unsigned char> m_dataBuffer;

		Texture m_texture;
		
		virtual bool unload() override;
	public:
		using IAsset::IAsset;
		static AssetTypeID type() { return s_typeID; }


		virtual bool importFromFile(const std::filesystem::path& path) override;

		virtual bool load(AssetLoadFlags flags = 0) override;
		virtual bool write(AssetWriteFlags flags = 0) override;

		const Texture& getTexture() const;

	};
}