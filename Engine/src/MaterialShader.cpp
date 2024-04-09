#include "pch.h"
#include "Assets/MaterialShader.h"

#include "Engine.h"

namespace sa {

    void MaterialShader::create(const std::vector<ShaderSourceFile>& sourceFiles) {
        if (sourceFiles.empty()) {
            throw std::runtime_error("Invalid sources");
        }
        m_sourceFiles = sourceFiles;
        for (auto& source : m_sourceFiles) {
            std::filesystem::path path = std::filesystem::current_path();
            std::filesystem::current_path(Engine::getShaderDirectory());
            std::vector<uint32_t> code = CompileGLSLFromFile(
                source.filePath.generic_string().c_str(),
                source.stage,
                "main",
                sa::Engine::getShaderDirectory().generic_string().c_str());

            m_code.push_back(code);
            std::filesystem::current_path(path);
        }
        create();
    }

    void MaterialShader::create(const std::vector<std::vector<uint32_t>>& sourceCode) {
        m_code = sourceCode;
        create();
    }

    void MaterialShader::create() {
        if (m_colorPipelineLayout.isValid())
            m_colorPipelineLayout.destroy();

        if (m_depthPipelineLayout.isValid())
            m_depthPipelineLayout.destroy();

        m_shaders.reserve(m_code.size());
        for (const auto& code : m_code) {
            Shader shader;
            shader.create(code);
            m_shaders.push_back(shader);
        }

        m_colorPipelineLayout.createFromShaders(m_shaders);
        m_depthPipelineLayout.createFromShaders({ m_shaders[0] });
        m_recompiled = true;
    }

    void MaterialShader::recreatePipelines(ResourceID colorRenderProgram, ResourceID depthRenderProgram) {
        if (!m_recompiled)
            return;

        auto& renderer = Renderer::get();
        if (m_depthPipeline != NULL_RESOURCE)
            renderer.destroyPipeline(m_depthPipeline);

        if (m_colorPipeline != NULL_RESOURCE)
            renderer.destroyPipeline(m_colorPipeline);


        PipelineSettings settings = {};
        settings.dynamicStates.push_back(VIEWPORT);
        settings.dynamicStates.push_back(SCISSOR);

        m_colorPipeline = renderer.createGraphicsPipeline(
            m_colorPipelineLayout,
            m_shaders.data(),
            m_shaders.size(),
            colorRenderProgram,
            0,
            { 0, 0 },
            settings);

        // Dynamic states neede to be used for shadows
        settings.dynamicStates.push_back(DynamicState::DEPTH_BIAS);
        settings.dynamicStates.push_back(DynamicState::DEPTH_BIAS_ENABLE);
        settings.dynamicStates.push_back(DynamicState::CULL_MODE);

        m_depthPipeline = renderer.createGraphicsPipeline(
            m_depthPipelineLayout,
            &m_shaders[0], // assuming the first shader is the vertex shader
            1,
            depthRenderProgram,
            0,
            { 0, 0 },
            settings);

        m_recompiled = false;
        SA_DEBUG_LOG_INFO("Recreated pipelines, MaterialShader UUID: ", getID());
    }

    bool MaterialShader::arePipelinesReady() const {
        return !m_recompiled && m_colorPipeline != NULL_RESOURCE && m_depthPipeline != NULL_RESOURCE;
    }

    void MaterialShader::bindDepthPipeline(RenderContext& context) const {
        context.bindPipelineLayout(m_depthPipelineLayout);
        context.bindPipeline(m_depthPipeline);
    }

    void MaterialShader::bindColorPipeline(RenderContext& context) const {
        context.bindPipelineLayout(m_colorPipelineLayout);
        context.bindPipeline(m_colorPipeline);
    }

    PipelineLayout& MaterialShader::getDepthPipelineLayout() {
        return m_depthPipelineLayout;
    }

    PipelineLayout& MaterialShader::getColorPipelineLayout() {
        return m_colorPipelineLayout;
    }

    const std::vector<ShaderSourceFile>& MaterialShader::getShaderSourceFiles() const {
        return m_sourceFiles;
    }

    std::vector<ShaderSourceFile>& MaterialShader::getShaderSourceFiles() {
        return m_sourceFiles;
    }

    void MaterialShader::addShaderSourceFile(const ShaderSourceFile& sourceFile) {
        m_sourceFiles.push_back(sourceFile);
    }

    void MaterialShader::removeShaderSourceFile(ShaderStageFlagBits stage) {
        auto it = std::find_if(m_sourceFiles.begin(), m_sourceFiles.end(), [&](const ShaderSourceFile& sourceFile) { return sourceFile.stage == stage; });
        if (it != m_sourceFiles.end()) {
            m_sourceFiles.erase(it);
        }
    }

    void MaterialShader::compileSource() {
        create(m_sourceFiles);
    }

    bool MaterialShader::isCompiled() const {
        return !m_recompiled && !m_shaders.empty();
    }

    bool MaterialShader::onLoad(std::ifstream& file, AssetLoadFlags flags) {
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
            if (!m_sourceFiles.empty()) {
                SA_DEBUG_LOG_INFO("Compiling Material Shader asset ", getName(), " from source files");
                for (auto& source : m_sourceFiles) {
                    SA_DEBUG_LOG_INFO(to_string(source.stage), " Stage : ", source.filePath);
                }
                create(m_sourceFiles);
            }
            return true;
        }

        create();

        return true;
    }

    bool MaterialShader::onWrite(std::ofstream& file, AssetWriteFlags flags) {

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

    bool MaterialShader::onUnload() {
        for (auto& shader : m_shaders) {
            shader.destroy();
        }
        m_code.clear();
        m_code.shrink_to_fit();
        m_shaders.clear();
        m_shaders.shrink_to_fit();
        m_sourceFiles.clear();
        m_sourceFiles.shrink_to_fit();

        m_colorPipelineLayout.destroy();
        m_depthPipelineLayout.destroy();
        return true;
    }
}