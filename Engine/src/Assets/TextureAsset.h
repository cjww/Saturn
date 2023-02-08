#pragma once

#include "IAsset.h"
#include "Resources/Texture.hpp"

namespace sa {
	class TextureAsset : public IAsset {
	private:
		Texture m_texture;
	public:
		using IAsset::IAsset;

		virtual bool importFromFile(const std::filesystem::path& path) override;

		virtual bool load() override;
		virtual bool write() override;

		const Texture& getTexture() const;

	};
}