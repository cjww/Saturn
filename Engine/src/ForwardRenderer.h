#pragma once
#include "IRenderTechnique.h"

#include <Renderer.hpp>
#include "functions.hpp"

#include "ECS\ECSCoordinator.h"
#include "ECS\Systems\Rendering\Vulkan\MeshRenderSystem.h"
#include "ECS\Systems\Rendering\Vulkan\DescriptorCreationSystem.h"

class ForwardRenderer : public IRenderTechnique {
private:
	vr::Renderer* m_renderer;

	vr::ShaderSetPtr m_pShaderSet;

	uint32_t m_frameBuffer;
	vr::Texture* m_pDepthTexture;

	uint32_t m_renderPass;
	uint32_t m_pipeline;
	
	//Systems
	vk::MeshRenderSystem* m_pMeshRenderSystem;
	vk::DescriptorCreationSystem* m_pDescriptorCreationSystem;

	bool beginFrame();
	void endFrame();
public:
	ForwardRenderer();

	virtual void init(RenderWindow* pWindow, bool setupImGui) override;
	virtual void cleanup() override;

	virtual void beginFrameImGUI() override;

	virtual void draw() override;

};