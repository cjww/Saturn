#include "pch.h"
#include "TextureAsset.h"

#include "Tools/Logger.hpp"
#include "AssetManager.h"


namespace sa {

    bool TextureAsset::importFromFile(const std::filesystem::path& path) {
        try {
            std::ifstream file(path, std::ios::binary | std::ios::ate);
            if (!file.good()) {
                return false;
            }
            int size = file.tellg();
            m_dataBuffer.resize(size);
            file.seekg(0);
            file.read((char*)m_dataBuffer.data(), size);

            file.close();

            Image img(m_dataBuffer.data(), m_dataBuffer.size());
            m_texture = Texture2D(img, true);
        }
        catch (const std::exception& e) {
            SA_DEBUG_LOG_ERROR(e.what());
            return false;
        }
        m_header.type = type();
        m_isLoaded = true;
        return true;
    }

    bool TextureAsset::load() {
        return dispatchLoad([&](std::ifstream& file) {

            m_dataBuffer.resize(m_header.size);

            file.read((char*)m_dataBuffer.data(), m_dataBuffer.size());

            if (m_texture.isValid())
                m_texture.destroy();

            Image img(m_dataBuffer.data(), m_dataBuffer.size());
            m_texture = Texture2D(img, true);

            return true;
        });
    }

    bool TextureAsset::write() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_header.size = m_dataBuffer.size();
        return dispatchWrite([&](std::ofstream& file) {
            if(m_dataBuffer.size() > 0)
                file.write((char*)m_dataBuffer.data(), m_dataBuffer.size());
            return true;
        });
    }

    const Texture& TextureAsset::getTexture() const {
        return m_texture;
    }
}