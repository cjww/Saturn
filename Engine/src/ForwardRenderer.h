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

		vr::ShaderSetPtr m_pColorShaders;
		vr::ShaderSetPtr m_pPostProcessShaders;

		vr::DescriptorSetPtr m_pPerFrameDescriptorSet;
		vr::Buffer* m_pPerFrameBuffer;

		vr::DescriptorSetPtr m_pInputDescriptorSet;

		uint32_t m_mainFramebuffer;
		vr::Texture* m_pDepthTexture;
		vr::Texture* m_pMainColorTexture;
		vr::Texture* m_pOutputTexture;

		uint32_t m_renderPass;
		
		uint32_t m_colorPipeline;
		uint32_t m_postProcessPipline;
	
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

		virtual vr::Texture* getOutputTexture() const override;

		virtual vr::Texture* createShaderTexture2D(const vr::Image& img) override;
		
	};

}