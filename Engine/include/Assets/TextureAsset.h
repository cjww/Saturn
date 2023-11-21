#pragma once

#include <Resources/Texture.hpp>
#include "Asset.h"

namespace sa {
	class TextureAsset : public Asset {
	private:
		std::vector<unsigned char> m_dataBuffer;

		Texture m_texture;
		
	public:
		using Asset::Asset;
		
		virtual bool onImport(const std::filesystem::path& path) override;

		virtual bool onLoad(std::ifstream& file, AssetLoadFlags flags) override;
		virtual bool onWrite(std::ofstream& file, AssetWriteFlags flags) override;
		virtual bool onUnload() override;


		const Texture& getTexture() const;

	};
}