#include "pch.h"
#include "Graphics/DebugRenderer.h"
#include "Engine.h"

#include "Shader.hpp"

namespace sa {

    DebugRenderer::DebugRenderer() 
        : m_isInitialized(false)
        , m_pipeline(NULL_RESOURCE)
    {
        m_shaders[0].create(ReadSPVFile((Engine::getShaderDirectory() / "DebugDrawing.vert.spv").generic_string().c_str()));
        m_shaders[1].create(ReadSPVFile((Engine::getShaderDirectory() / "DebugDrawing.frag.spv").generic_string().c_str()));

        m_pipelineLayout.createFromShaders(m_shaders.data(), m_shaders.size());


        m_lineVertexBuffer.create(BufferType::VERTEX);
    }


    DebugRenderer& DebugRenderer::Get() {
        static DebugRenderer instance;
        return instance;
    }

    void DebugRenderer::initialize(ResourceID renderProgram) {
        
        PipelineSettings settings = {};
        settings.cullMode = CullModeFlagBits::NONE;
        settings.depthTestEnabled = false;
        settings.dynamicStates.push_back(DynamicState::VIEWPORT);
        settings.dynamicStates.push_back(DynamicState::SCISSOR);

        m_pipeline = Renderer::get().createGraphicsPipeline(m_pipelineLayout, m_shaders.data(), m_shaders.size(), renderProgram, 0, { 0, 0 }, settings);

        settings.polygonMode = PolygonMode::LINE;
        settings.topology = Topology::LINE_LIST;
        m_wireframePipeline = Renderer::get().createGraphicsPipeline(m_pipelineLayout, m_shaders.data(), m_shaders.size(), renderProgram, 0, { 0, 0 }, settings);
        
        m_isInitialized = true;
    }

    bool DebugRenderer::isInitialized() const {
        return m_isInitialized;
    }

    void DebugRenderer::render(RenderContext& context, Extent extent, const SceneCamera& sceneCamera) {
        if(!m_lineList.empty())
            m_lineVertexBuffer.write(m_lineList);
            
        context.bindPipelineLayout(m_pipelineLayout);
        context.bindPipeline(m_wireframePipeline);

        Rect viewport = Rect{
            .offset = { 0, 0 },
            .extent = extent
        };
        context.setViewport(viewport);
        context.setScissor(viewport);

        glm::mat4 mat = sceneCamera.getProjectionMatrix() * sceneCamera.getViewMatrix();
        context.pushConstant(ShaderStageFlagBits::VERTEX, mat);

        context.bindVertexBuffers(0, &m_lineVertexBuffer.getBuffer(), 1);
        
        context.draw(m_lineVertexBuffer.getElementCount<Vertex>(), 1);

        m_lineVertexBuffer.swap();
        m_lineList.clear();
    }

    void DebugRenderer::drawLine(const glm::vec3& p1, const glm::vec3& p2, Color color) {
        m_lineList.push_back(Vertex{
            .position = p1,
            .color = color
        });
        m_lineList.push_back(Vertex{
            .position = p2,
            .color = color
        });
    }

    void DebugRenderer::drawLines(glm::vec3* pPoints, uint32_t pointCount, Color color) {
        for (uint32_t i = 0; i < pointCount - 1; i++) {
            m_lineList.push_back(Vertex{
                .position = pPoints[i],
                .color = color 
            });
            m_lineList.push_back(Vertex{
                .position = pPoints[i + 1],
                .color = color
            });
        }
    }

    void DebugRenderer::drawLineLoop(glm::vec3* pPoints, uint32_t pointCount, Color color) {
        for (uint32_t i = 0; i < pointCount; i++) {
            m_lineList.push_back(Vertex{
                .position = pPoints[i],
                .color = color
            });
            m_lineList.push_back(Vertex{
                .position = pPoints[(i + 1) % pointCount],
                .color = color
            });
        }
    }


}