#include "pch.h"
#include "Assets/TextureAsset.h"

#include "Tools/Logger.hpp"
#include "AssetManager.h"


namespace sa {

    bool TextureAsset::onImport(const std::filesystem::path& path) {
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
            m_texture.create2D(img, true);

            AssetHeader header = getHeader();
            header.size = m_dataBuffer.size();
            setHeader(header);
        }
        catch (const std::exception& e) {
            SA_DEBUG_LOG_ERROR(e.what());
            return false;
        }
        return true;
    }

    bool TextureAsset::onLoad(JsonObject& metaData, AssetLoadFlags flags) {
        setCompletionCount(2);
        
        if (m_texture.isValid())
            m_texture.destroy();

        Image img(getAssetPath().generic_string().c_str());
        incrementProgress();
        m_texture.create2D(img, true);
        incrementProgress();
        return true;
    }

    bool TextureAsset::onLoadCompiled(ByteStream& dataInStream, AssetLoadFlags flags) {
        setCompletionCount(3);
        m_dataBuffer.resize(getHeader().size);
        
        dataInStream.read(static_cast<byte_t*>(m_dataBuffer.data()), m_dataBuffer.size());
        incrementProgress();

        if (m_texture.isValid())
            m_texture.destroy();

        Image img(m_dataBuffer.data(), m_dataBuffer.size());
        incrementProgress();
        m_texture.create2D(img, true);
        incrementProgress();
        AssetHeader header = getHeader();
        header.size = m_dataBuffer.size(); // update size
        setHeader(header);
        return true;
    }

    bool TextureAsset::onWrite(AssetWriteFlags flags) {
        return false;
    }

    bool TextureAsset::onCompile(ByteStream& dataOutStream, AssetWriteFlags flags) {
        setCompletionCount(1);
        if (m_dataBuffer.size() > 0)
            dataOutStream.write(m_dataBuffer.data(), m_dataBuffer.size());
        incrementProgress();
        return true;
    }

    bool TextureAsset::onUnload() {
        m_dataBuffer.clear();
        m_dataBuffer.shrink_to_fit();
        m_texture.destroy();
        return true;
    }

    const Texture& TextureAsset::getTexture() const {
        return m_texture;
    }
}