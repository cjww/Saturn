#pragma once
#include "Graphics/IRenderTechnique.h"

namespace sa {
	class ForwardPlus : public IRenderTechnique {
	private:

		// Color pass
		Texture2D m_colorTexture;
		Texture2D m_depthTexture;

		ResourceID m_colorRenderProgram = NULL_RESOURCE;
		ResourceID m_colorFramebuffer = NULL_RESOURCE;
		ResourceID m_colorPipeline = NULL_RESOURCE;

		ResourceID m_composeRenderProgram = NULL_RESOURCE;
		ResourceID m_composeFramebuffer = NULL_RESOURCE;
		ResourceID m_composePipeline = NULL_RESOURCE;

		ResourceID m_composeDescriptorSet = NULL_RESOURCE;
		Texture2D m_outputTexture; // if using Imgui

		ResourceID m_sceneDescriptorSet = NULL_RESOURCE;
		Buffer m_sceneUniformBuffer;
		Buffer m_lightBuffer;
		
		ResourceID m_linearSampler = NULL_RESOURCE;

		void createTextures(Extent extent);
		void createRenderPasses();
		void createFramebuffers(Extent extent);
		void createPipelines(Extent extent);

	public:

		virtual void init(sa::RenderWindow* pWindow, bool setupImGui) override;
		virtual void cleanup() override;

		virtual void beginFrameImGUI() override;

		virtual void draw(Scene* scene) override;

		virtual Texture getOutputTexture() const override;

		virtual void updateLights(Scene* pScene) override;


	};
}

