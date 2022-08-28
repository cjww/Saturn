#pragma once
#include "Graphics/IRenderTechnique.h"

namespace sa {
	class ForwardPlus : public IRenderTechnique {
	private:

		Texture2D m_outputTexture;
		Texture2D m_depthTexture;
		Texture2D m_colorTexture;

		ResourceID m_colorRenderProgram = NULL_RESOURCE;
		ResourceID m_colorFramebuffer = NULL_RESOURCE;
		ResourceID m_colorPipeline = NULL_RESOURCE;

		Buffer m_sceneUniformBuffer;
		ResourceID m_sceneDescriptorSet = NULL_RESOURCE;
		
		ResourceID m_linearSampler = NULL_RESOURCE;

		Buffer m_lightBuffer;

		void createTextures(Extent extent);
		void createRenderPasses();
		void createFramebuffers(Extent extent);

	public:

		virtual void init(Extent extent, bool setupImGui = false) override;
		virtual void cleanup() override;

		virtual void beginFrameImGUI() override;

		virtual void draw(const Scene* scene) override;
		virtual Texture getOutputTexture() const override;

		virtual void updateLights(Scene* pScene) override;


	};
}

