#pragma once

#include "Assets/ModelAsset.h"
#include "Graphics/IRenderLayer.h"

#include "Resources/DynamicTexture.hpp"
#include "Resources/DynamicBuffer.hpp"

#include "Graphics\RenderPipeline.h"

#include "Tools/Profiler.h"

#define TILE_SIZE 16U
#define MAX_LIGHTS_PER_TILE 1024


namespace sa {

	class ShadowRenderLayer;
	class EnvironmentRenderLayer;

	struct ForwardPlusRenderData {
		DynamicTexture colorTexture;
		DynamicTexture depthTexture;
		ResourceID colorFramebuffer = NULL_RESOURCE;
		ResourceID depthFramebuffer = NULL_RESOURCE;


		// Light culling
		glm::uvec2 tileCount;
		DynamicBuffer lightIndexBuffer;
		ResourceID lightCullingDescriptorSet = NULL_RESOURCE;

		DynamicTexture debugLightHeatmap;
		ResourceID debugLightHeatmapFramebuffer = NULL_RESOURCE;
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

		PipelineLayout m_lightCullingLayout;
		Shader m_lightCullingShader;

		PipelineLayout m_debugHeatmapLayout;
		Shader m_debugHeatmapVertexShader;
		Shader m_debugHeatmapFragmentShader;
		Format m_debugTextureFormat;
		ResourceID m_debugLightHeatmapRenderProgram = NULL_RESOURCE;
		ResourceID m_debugLightHeatmapPipeline = NULL_RESOURCE;

		ShadowRenderLayer* m_pShadowRenderLayer;
		Buffer m_defaultShadowPreferencesBuffer;
		Buffer m_defaultShadowDataBuffer;

		struct {
			Texture cubemap;
			PipelineLayout pipelineLayout;
			ResourceID pipeline;
			Buffer vertexBuffer;
			Buffer indexBuffer;
			ResourceID descriptorSet;
		} m_skybox;


		void createPreDepthPass();
		void createLightCullingShader();
		void createColorPass();

		void createSkyboxPipeline();

		void initializeMainRenderData(ForwardPlusRenderData& data, Extent extent);
		void cleanupMainRenderData(ForwardPlusRenderData& data);

		void bindShadows(const RenderContext& context, const SceneCollection& sc, const MaterialShaderCollection& collection);

	public:

		ForwardPlus(const RenderPipeline& renderPipeline);

		virtual void onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent) override;

		virtual void init() override;
		virtual void cleanup() override;

		virtual bool render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;
		virtual bool postRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;

	};
}

