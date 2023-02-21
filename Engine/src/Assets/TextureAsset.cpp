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
        m_isLoaded = true;
        return true;
    }

    bool TextureAsset::load(AssetLoadFlags flags) {
        return dispatchLoad([&](std::ifstream& file) {
            m_progress.setMaxCompletionCount(3);
            m_dataBuffer.resize(m_header.size);

            file.read((char*)m_dataBuffer.data(), m_dataBuffer.size());
            m_progress.increment();

            if (m_texture.isValid())
                m_texture.destroy();

            Image img(m_dataBuffer.data(), m_dataBuffer.size());
            m_progress.increment();
            m_texture = Texture2D(img, true);
            m_progress.increment();

            return true;
        }, flags);
    }

    bool TextureAsset::write(AssetWriteFlags flags) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_header.size = m_dataBuffer.size();
        return dispatchWrite([&](std::ofstream& file) {
            m_progress.setMaxCompletionCount(1);
            if(m_dataBuffer.size() > 0)
                file.write((char*)m_dataBuffer.data(), m_dataBuffer.size());
            m_progress.increment();
            return true;
        }, flags);
    }

    bool TextureAsset::unload() {
        m_dataBuffer.clear();
        m_dataBuffer.shrink_to_fit();
        m_texture.destroy();
        m_isLoaded = false;
        return true;
    }

    const Texture& TextureAsset::getTexture() const {
        return m_texture;
    }
}