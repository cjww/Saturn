#pragma once
#include "IRenderTechnique.h"

#include "structs.h"
#include "ECS\Components.h"


namespace sa {
	class ForwardRenderer : public IRenderTechnique {
	private:
		

		ResourceID m_colorRenderProgram;
		ResourceID m_colorPipeline;
		ResourceID m_colorFramebuffer;
		
		ResourceID m_postRenderProgram;
		ResourceID m_postProcessPipeline;
		ResourceID m_postFramebuffer;
		ResourceID m_postInputDescriptorSet;

		ResourceID m_blurPipeline;
		ResourceID m_blurDescriptorSet;
		SubContext m_blurContext;

		ResourceID m_perFrameDescriptorSet;
		ResourceID m_materialDescriptorSet;

		Buffer m_perFrameBuffer;


		ResourceID m_imguiRenderProgram;
		ResourceID m_imguiFramebuffer;

		Texture2D m_depthTexture;
		Texture2D m_mainColorTexture;

		Texture2D m_brightnessTexture;
		Texture2D m_blurredBrightnessTexture;
		
		Texture2D m_outputTexture;
		Texture2D m_resolveTexture;

		ResourceID m_sampler;
		
		Buffer m_lightBuffer;
		
		void setupColorPass(Extent extent);
		void setupPostPass(Extent extent);
		void setupBlurPass();
		
	public:
		
		void swapchainResizedCallback(Extent width);


		virtual void init(sa::RenderWindow* pWindow, bool setupImGui) override;
		virtual void cleanup() override;

		virtual void beginFrameImGUI() override;

		virtual void draw(Scene* scene) override;

		virtual sa::Texture getOutputTexture() const override;

		virtual void updateLights(Scene* pScene) override;
	};

}