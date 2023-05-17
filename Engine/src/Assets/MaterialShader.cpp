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
    m_currentExtent = { 0, 0 };
    m_colorShaderSet.create(m_code);
    m_depthShaderSet.create({ m_code[0] });
    
}

void sa::MaterialShader::create(const std::vector<std::vector<uint32_t>>& code) {
    if (m_colorShaderSet.isValid())
        m_colorShaderSet.destroy();

    if (m_depthShaderSet.isValid())
        m_depthShaderSet.destroy();

    m_code = code;
    m_colorShaderSet.create(m_code);
    m_depthShaderSet.create({ m_code[0] });
    m_currentExtent = { 0, 0 };
}

void sa::MaterialShader::recreatePipelines(ResourceID colorRenderProgram, ResourceID depthRenderProgram, Extent extent) {
    if (extent == m_currentExtent)
        return;
    
    PipelineSettings settings = {};
    settings.dynamicStates.push_back(sa::VIEWPORT);

    auto& renderer = Renderer::get();
    if (m_colorPipeline != NULL_RESOURCE) {
        renderer.destroyPipeline(m_colorPipeline);
    }
    if (m_depthPipeline != NULL_RESOURCE) {
        renderer.destroyPipeline(m_depthPipeline);
    }

    m_colorPipeline = renderer.createGraphicsPipeline(colorRenderProgram, 0, extent, m_colorShaderSet, settings);
    m_depthPipeline = renderer.createGraphicsPipeline(depthRenderProgram, 0, extent, m_depthShaderSet, settings);
    m_currentExtent = extent;

}

void sa::MaterialShader::bindColorPipeline(RenderContext& context) {
    context.bindPipeline(m_colorPipeline);
}

void sa::MaterialShader::bindDepthPipeline(RenderContext& context) {
    context.bindPipeline(m_depthPipeline);
}

const std::vector<sa::ShaderSourceFile>& sa::MaterialShader::getShaderSourceFiles() const {
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
    uint32_t codeCount = 0;
    file.read((char*)&codeCount, sizeof(uint32_t));
    m_code.resize(codeCount);
    m_sourceFiles.resize(codeCount);
    //Source files
    for (int i = 0; i < codeCount; i++) {
        uint32_t pathLength = 0;
        file.read((char*)&pathLength, sizeof(uint32_t));
        std::string path;
        path.resize(pathLength);
        file.read(path.data(), path.size());
        m_sourceFiles[i].filePath = path;
        file.read((char*)&m_sourceFiles[i].stage, sizeof(uint32_t));
    }

    for (int i = 0; i < codeCount; i++) {
        size_t codeSize = 0;
        file.read((char*)&codeSize, sizeof(size_t));
        m_code[i].resize(codeSize);
        file.read((char*)m_code[i].data(), codeSize * sizeof(uint32_t));
    }

    try {
        create(m_code);
    }
    catch (const std::exception& e) {
        SA_DEBUG_LOG_ERROR(e.what());
        return false;
    }

    return true;
}

bool sa::MaterialShader::onWrite(std::ofstream& file, AssetWriteFlags flags) {
    uint32_t codeCount = m_code.size();
    file.write((char*)&codeCount, sizeof(uint32_t));
    
    //Source files
    for (auto& source : m_sourceFiles) {
        uint32_t pathLength = source.filePath.generic_string().length();
        file.write((char*)&pathLength, sizeof(uint32_t));
        file.write(source.filePath.generic_string().c_str(), pathLength);
        uint32_t stage = source.stage;
        file.write((char*)&stage, sizeof(uint32_t));
    }
    //Binary code
    for (auto& code : m_code) {
        size_t codeSize = code.size();
        file.write((char*)&codeSize, sizeof(size_t));
        file.write((char*)code.data(), code.size() * sizeof(uint32_t));
    }
    return true;
}

bool sa::MaterialShader::onUnload() {
    m_code.clear();
    return true;
}
