#include "pch.h"
#include "MaterialShader.h"

sa::MaterialShader::MaterialShader() {

}

void sa::MaterialShader::create(const std::vector<std::vector<uint32_t>>& code) {
    m_colorShaderSet.create(code);
    m_depthShaderSet.create({ code[0] });
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
/*
bool sa::MaterialShader::onLoad(std::ifstream& file, AssetLoadFlags flags)
{
    return false;
}

bool sa::MaterialShader::onWrite(std::ofstream& file, AssetWriteFlags flags) {

    return false;
}

bool sa::MaterialShader::onUnload() {

    return false;
}

*/