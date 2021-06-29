#pragma once
#include "IRenderTechnique.h"

#include <Renderer.hpp>
#include "functions.hpp"

#include "ECS\ECSCoordinator.h"
#include "ECS\Systems\Rendering\Vulkan\MeshRenderSystem.h"
#include "ECS\Systems\Rendering\Vulkan\DescriptorCreationSystem.h"
#include "ECS\Systems\Rendering\Vulkan\DescriptorManager.h"

namespace sa {
	class ForwardRenderer : public IRenderTechnique {
	private:
		vr::Renderer* m_renderer;

		vr::ShaderSetPtr m_pShaderSet;


		vr::DescriptorSetPtr m_pPerFrameDescriptorSet;
		vr::Buffer* m_pPerFrameBuffer;


		uint32_t m_frameBuffer;
		vr::Texture* m_pDepthTexture;

		uint32_t m_renderPass;
		uint32_t m_pipeline;
	
		vk::DescriptorManager* m_pDescriptorManager;

		//Systems
		vk::MeshRenderSystem* m_pMeshRenderSystem;
		vk::DescriptorCreationSystem* m_pDescriptorCreationSystem;

	public:
		ForwardRenderer();

		virtual void init(RenderWindow* pWindow, bool setupImGui) override;
		virtual void cleanup() override;

		virtual void beginFrameImGUI() override;

		virtual void draw() override;

	};

}