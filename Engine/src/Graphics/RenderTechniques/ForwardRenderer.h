#pragma once
#include "Graphics/IRenderTechnique.h"

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
		using IRenderTechnique::IRenderTechnique;
		virtual void onWindowResize(Extent extent) override;


		virtual void init(sa::RenderWindow* pWindow, IRenderLayer* = nullptr) override;
		virtual void cleanup() override;

		virtual void updateData(RenderContext& context, Scene* pScene) override;
		virtual void preRender(RenderContext& context, Camera* pCamera) override;
		virtual void render(RenderContext& context, Camera* pCamera) override;
		virtual void postRender(RenderContext& context) override;

		virtual void updateLights(Scene* pScene) override;
	};

}