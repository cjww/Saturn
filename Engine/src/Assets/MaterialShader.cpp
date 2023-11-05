#include "pch.h"
#include "MaterialShader.h"

#include "Engine.h"

void sa::MaterialShader::create(const std::vector<ShaderSourceFile>& sourceFiles) {
    if (m_colorShaderSet.isValid())
        m_colorShaderSet.destroy();

    if (m_depthShaderSet.isValid())
        m_depthShaderSet.destroy();

    m_sourceFiles = sourceFiles;
    for (auto& source : m_sourceFiles) {
        std::filesystem::path path = std::filesystem::current_path();
        std::filesystem::current_path(sa::Engine::getShaderDirectory());
        m_code.push_back(sa::CompileGLSLFromFile(source.filePath.generic_string().c_str(), source.stage, "main", source.filePath.generic_string().c_str()));
        std::filesystem::current_path(path);
    }
    m_colorShaderSet.create(m_code);
    m_depthShaderSet.create({ m_code[0] });
    m_recompiled = true;
}

void sa::MaterialShader::create(const std::vector<std::vector<uint32_t>>& code) {
    if (m_colorShaderSet.isValid())
        m_colorShaderSet.destroy();

    if (m_depthShaderSet.isValid())
        m_depthShaderSet.destroy();

    m_code = code;
    m_colorShaderSet.create(m_code);
    m_depthShaderSet.create({ m_code[0] });
    m_recompiled = true;
}

const std::vector<sa::ShaderSourceFile>& sa::MaterialShader::getShaderSourceFiles() const {
    return m_sourceFiles;
}

std::vector<sa::ShaderSourceFile>& sa::MaterialShader::getShaderSourceFiles() {
    return m_sourceFiles;
}

void sa::MaterialShader::addShaderSourceFile(const sa::ShaderSourceFile& sourceFile) {
    m_sourceFiles.push_back(sourceFile);
}

void sa::MaterialShader::removeShaderSourceFile(sa::ShaderStageFlagBits stage) {
    auto it = std::find_if(m_sourceFiles.begin(), m_sourceFiles.end(), [&](const ShaderSourceFile& sourceFile) { return sourceFile.stage == stage; });
    if (it != m_sourceFiles.end()) {
        m_sourceFiles.erase(it);
    }
}

void sa::MaterialShader::compileSource() {
    create(m_sourceFiles);
}

bool sa::MaterialShader::onLoad(std::ifstream& file, AssetLoadFlags flags) {
    //Source files
    uint32_t sourceCount = 0;
    file.read((char*)&sourceCount, sizeof(uint32_t));

    m_sourceFiles.resize(sourceCount);
    for (int i = 0; i < sourceCount; i++) {
        uint32_t pathLength = 0;
        file.read((char*)&pathLength, sizeof(uint32_t));
        std::string path;
        path.resize(pathLength);
        file.read(path.data(), path.size());
        m_sourceFiles[i].filePath = path;
        file.read((char*)&m_sourceFiles[i].stage, sizeof(uint32_t));
    }
    // Binary code
    uint32_t codeCount = 0;
    file.read((char*)&codeCount, sizeof(uint32_t));

    m_code.resize(codeCount);
    for (int i = 0; i < codeCount; i++) {
        size_t codeSize = 0;
        file.read((char*)&codeSize, sizeof(size_t));
        m_code[i].resize(codeSize);
        file.read((char*)m_code[i].data(), codeSize * sizeof(uint32_t));
    }

    if (m_code.empty()) {
        SA_DEBUG_LOG_WARNING("No spv code read from Material Shader asset ", getName());
        if(m_sourceFiles.empty())
            return false;
        SA_DEBUG_LOG_INFO("Compiling Material Shader asset ", getName(), " from source files");
        for (auto& source : m_sourceFiles) {
            SA_DEBUG_LOG_INFO(sa::to_string(source.stage), " Stage : ", source.filePath);
        }
        create(m_sourceFiles);
        return true;
    }

    create(m_code);

    return true;
}

bool sa::MaterialShader::onWrite(std::ofstream& file, AssetWriteFlags flags) {

    //Source files
    uint32_t sourceCount = m_sourceFiles.size();
    file.write((char*)&sourceCount, sizeof(uint32_t));

    for (auto& source : m_sourceFiles) {
        uint32_t pathLength = source.filePath.generic_string().length();
        file.write((char*)&pathLength, sizeof(uint32_t));
        file.write(source.filePath.generic_string().c_str(), pathLength);
        uint32_t stage = source.stage;
        file.write((char*)&stage, sizeof(uint32_t));
    }

    //Binary code
    uint32_t codeCount = m_code.size();
    file.write((char*)&codeCount, sizeof(uint32_t));

    for (auto& code : m_code) {
        size_t codeSize = code.size();
        file.write((char*)&codeSize, sizeof(size_t));
        file.write((char*)code.data(), code.size() * sizeof(uint32_t));
    }
    return true;
}

bool sa::MaterialShader::onUnload() {
    m_code.clear();
    m_code.shrink_to_fit();
    m_sourceFiles.clear();
    m_sourceFiles.shrink_to_fit();

    m_colorShaderSet.destroy();
    m_depthShaderSet.destroy();
    return true;
}
