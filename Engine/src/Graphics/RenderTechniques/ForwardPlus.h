#pragma once
#include "Graphics\IRenderTechnique.h"

#include "Assets/ModelAsset.h"

#include "Resources/DynamicTexture.hpp"
#include "Resources/DynamicBuffer.hpp"

#include "Tools/Profiler.h"

#define TILE_SIZE 16U
#define MAX_LIGHTS_PER_TILE 1024


namespace sa {

	class ForwardPlus : public IRenderTechnique {
	private:
		Renderer& m_renderer;

		ResourceID m_colorRenderProgram = NULL_RESOURCE;
		
		ResourceID m_linearSampler = NULL_RESOURCE;
		ResourceID m_nearestSampler = NULL_RESOURCE;

		ResourceID m_depthPreRenderProgram = NULL_RESOURCE;
		ResourceID m_lightCullingPipeline = NULL_RESOURCE;
		
		ShaderSet m_lightCullingShader;

		ShaderSet m_debugHeatmapShaderSet;
		
		void createPreDepthPass();
		void createLightCullingShader();
		void createColorPass();

	public:


		ForwardPlus();

		virtual void init() override;
		virtual void cleanup() override;

		virtual bool preRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;
		virtual const Texture& render(RenderContext& context, SceneCamera* pCamera, RenderTarget* rendertarget, SceneCollection& sceneCollection) override;

	};
}

