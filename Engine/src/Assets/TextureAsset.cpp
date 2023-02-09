#include "pch.h"
#include "TextureAsset.h"

namespace sa {

    bool TextureAsset::importFromFile(const std::filesystem::path& path) {
        try {
            Image img(path.generic_string());
            m_texture = Texture2D(img, true);
        }
        catch (const std::exception& e) {
            return false;
        }
        m_header.type = AssetType::TEXTURE;
        m_isLoaded = true;
        return true;
    }

    bool TextureAsset::load() {
        return false;
    }

    bool TextureAsset::write() {
        return false;
    }

    const Texture& TextureAsset::getTexture() const {
        return m_texture;
    }
}