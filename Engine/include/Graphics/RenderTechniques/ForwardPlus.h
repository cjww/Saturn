#pragma once

#include "Assets/ModelAsset.h"
#include "Graphics/IRenderLayer.h"

#include "Resources/DynamicTexture.hpp"
#include "Resources/DynamicBuffer.hpp"

#include "Tools/Profiler.h"

#define TILE_SIZE 16U
#define MAX_LIGHTS_PER_TILE 1024


namespace sa {

	class ShadowRenderLayer;

	struct ForwardPlusRenderData {
		DynamicTexture colorTexture;
		DynamicTexture depthTexture;
		ResourceID colorFramebuffer = NULL_RESOURCE;
		ResourceID depthFramebuffer = NULL_RESOURCE;


		// Light culling
		glm::uvec2 tileCount;
		DynamicBuffer lightIndexBuffer;
		ResourceID lightCullingDescriptorSet = NULL_RESOURCE;

		DynamicTexture2D debugLightHeatmap;
		ResourceID debugLightHeatmapRenderProgram = NULL_RESOURCE;
		ResourceID debugLightHeatmapFramebuffer = NULL_RESOURCE;
		ResourceID debugLightHeatmapPipeline = NULL_RESOURCE;
		ResourceID debugLightHeatmapDescriptorSet = NULL_RESOURCE;
		bool renderDebugHeatmap = false;


		bool isInitialized = false;
	};

	struct ForwardPlusPreferences {
		
	};

	class ForwardPlus : public IRenderLayer<ForwardPlusRenderData, ForwardPlusPreferences> {
	private:
		ResourceID m_colorRenderProgram = NULL_RESOURCE;
		
		ResourceID m_linearSampler = NULL_RESOURCE;
		ResourceID m_nearestSampler = NULL_RESOURCE;

		ResourceID m_depthPreRenderProgram = NULL_RESOURCE;
		ResourceID m_lightCullingPipeline = NULL_RESOURCE;
		
		ShaderSet m_lightCullingShader;

		ShaderSet m_debugHeatmapShaderSet;

		ShadowRenderLayer* m_pShadowRenderLayer;

		void createPreDepthPass();
		void createLightCullingShader();
		void createColorPass();

		void initializeMainRenderData(ForwardPlusRenderData& data, Extent extent);
		void cleanupMainRenderData(ForwardPlusRenderData& data);

	public:

		ForwardPlus(ShadowRenderLayer* pShadowRenderLayer);

		virtual void onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent) override;

		virtual void init() override;
		virtual void cleanup() override;

		virtual bool render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;
		virtual bool postRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;

	};
}

