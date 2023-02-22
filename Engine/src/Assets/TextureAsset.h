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
		
	public:
		using IAsset::IAsset;
		static AssetTypeID type() { return s_typeID; }

		virtual bool onImport(const std::filesystem::path& path) override;

		virtual bool onLoad(std::ifstream& file, AssetLoadFlags flags) override;
		virtual bool onWrite(std::ofstream& file, AssetWriteFlags flags) override;
		virtual bool onUnload() override;


		const Texture& getTexture() const;

	};
}