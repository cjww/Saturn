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

		virtual bool onLoad(JsonObject& metaData, AssetLoadFlags flags) override;
		virtual bool onLoadCompiled(ByteStream& dataInStream, AssetLoadFlags flags) override;

		virtual bool onWrite(AssetWriteFlags flags) override;
		virtual bool onCompile(ByteStream& dataOutStream, AssetWriteFlags flags) override;
		virtual bool onUnload() override;


		const Texture& getTexture() const;

	};
}