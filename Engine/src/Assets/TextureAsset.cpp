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
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_isLoaded)
            return false;

        SA_DEBUG_LOG_INFO("Loading TextureAsset ", getID());

        std::ifstream file(m_assetPath, std::ios::binary);
        if (!file.good()) {
            file.close();
            return false;
        }
        m_header = readHeader(file);
        m_dataBuffer.resize(m_header.size);

        file.read((char*)m_dataBuffer.data(), m_dataBuffer.size());

        file.close();

        if (m_texture.isValid())
            m_texture.destroy();

        Image img(m_dataBuffer.data(), m_dataBuffer.size());
        m_texture = Texture2D(img, true);

        m_isLoaded = true;
        return true;
    }

    bool TextureAsset::write() {
        if (!m_isLoaded)
            return false;

        std::ofstream file(m_assetPath, std::ios::binary | std::ios::out);
        if (!file.good()) {
            file.close();
            return false;
        }
        m_header.size = m_dataBuffer.size();
        writeHeader(m_header, file);

        file.write((char*)m_dataBuffer.data(), m_dataBuffer.size());

        file.close();
        return true;
    }

    const Texture& TextureAsset::getTexture() const {
        return m_texture;
    }
}