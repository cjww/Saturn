#pragma once
#include "IRenderTechnique.h"

#include <Renderer.hpp>

#include "structs.h"
#include "ECS\Components.h"

namespace sa {
	class ForwardRenderer : public IRenderTechnique {
	private:
		vr::Renderer* m_renderer;

		vr::ShaderSetPtr m_pColorShaders;
		vr::ShaderSetPtr m_pPostProcessShaders;

		vr::DescriptorSetPtr m_pPerFrameDescriptorSet;
		vr::Buffer* m_pPerFrameBuffer;
		vr::Buffer* m_pLightBuffer;

		vr::DescriptorSetPtr m_pInputDescriptorSet;

		uint32_t m_mainFramebuffer;
		vr::Texture* m_pDepthTexture;
		vr::Texture* m_pMainColorTexture;
		vr::Texture* m_pOutputTexture;

		uint32_t m_renderPass;
		
		uint32_t m_colorPipeline;
		uint32_t m_postProcessPipline;
	

	public:
		ForwardRenderer();
		

		virtual void init(RenderWindow* pWindow, bool setupImGui) override;
		virtual void cleanup() override;

		virtual void beginFrameImGUI() override;

		virtual void draw(Scene* scene) override;

		virtual vr::Texture* getOutputTexture() const override;

		virtual vr::Texture* createShaderTexture2D(const vr::Image& img) override;
		
	};

}