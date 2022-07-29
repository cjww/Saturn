#pragma once
#include "IRenderTechnique.h"

#include "structs.h"
#include "ECS\Components.h"


namespace sa {
	class ForwardRenderer : public IRenderTechnique {
	private:
		sa::Renderer& m_renderer;


		ResourceID m_colorRenderProgram;
		ResourceID m_colorPipeline;
		ResourceID m_colorFramebuffer;

		ResourceID m_postRenderProgram;
		ResourceID m_postProcessPipeline;
		ResourceID m_postFramebuffer;
		ResourceID m_postInputDescriptorSet;

		ResourceID m_blurPipeline;
		ResourceID m_blurDescriptorSet;
		sa::SubContext m_blurContext;

		ResourceID m_perFrameDescriptorSet;

		sa::Buffer m_perFrameBuffer;
		sa::Buffer m_lightBuffer;


		ResourceID m_imguiRenderProgram;
		ResourceID m_imguiFramebuffer;

		sa::Texture2D m_depthTexture;
		sa::Texture2D m_mainColorTexture;

		sa::Texture2D m_brightnessTexture;
		sa::Texture2D m_blurredBrightnessTexture;
		
		sa::Texture2D m_outputTexture;
		
		ResourceID m_sampler;
		
		//DEBUG 
		sa::Texture2D m_boxTexture;
		
		float timer = 0.0f;

		void createTextures(sa::Extent extent);
		void createRenderPasses();
		void createFramebuffers(sa::Extent extent);
		
	public:
		ForwardRenderer();
		
		void swapchainResizedCallback(Extent width);


		virtual void init(sa::RenderWindow* pWindow, bool setupImGui) override;
		virtual void cleanup() override;

		virtual void beginFrameImGUI() override;

		virtual void draw(Scene* scene) override;

		virtual sa::Texture getOutputTexture() const override;

	};

}