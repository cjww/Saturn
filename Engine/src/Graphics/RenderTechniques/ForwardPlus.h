#pragma once
#include "Graphics/IRenderTechnique.h"

#include "Assets/ModelAsset.h"

#include "Resources/DynamicTexture.hpp"
#include "Resources/DynamicBuffer.hpp"

#include "Tools/Profiler.h"

#define TILE_SIZE 16U
#define MAX_LIGHTS_PER_TILE 1024


namespace sa {

	class ForwardPlus : public IRenderTechnique {
	private:


		ResourceID m_colorRenderProgram = NULL_RESOURCE;
		
		ResourceID m_linearSampler = NULL_RESOURCE;
		ResourceID m_nearestSampler = NULL_RESOURCE;

		ResourceID m_depthPreRenderProgram = NULL_RESOURCE;
		ResourceID m_lightCullingPipeline = NULL_RESOURCE;
		
		ResourceID m_debugLightHeatmapRenderProgram = NULL_RESOURCE;
		ResourceID m_debugLightHeatmapPipeline = NULL_RESOURCE;
		ResourceID m_debugLightHeatmapFramebuffer = NULL_RESOURCE;
		Texture2D m_debugLightHeatmap;
		ResourceID m_debugLightHeatmapDescriptorSet = NULL_RESOURCE;
		
		void createPreDepthPass();
		void createLightCullingShader();
		void createColorPass();

		void initializeMainRenderData(RenderTarget::MainRenderData& data, Extent extent);
		
	public:
		using IRenderTechnique::IRenderTechnique;

		virtual void onWindowResize(Extent extent) override;

		virtual void init() override;
		virtual void cleanup() override;

		virtual void updateData(RenderContext& context) override;
		virtual bool prepareRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;
		virtual void render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;
		
		const Texture2D& getLightHeatmap() const;
		void setShowHeatmap(bool value);

	};
}

