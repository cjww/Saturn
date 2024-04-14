#pragma once

#include "../IRenderLayer.h"

#define MAX_SHADOW_TEXTURE_COUNT 8u

namespace sa {


	struct ShadowPreferences {
		uint32_t directionalMapResolution = 4096u;
		uint32_t omniMapResolution = 1024u;

		uint32_t cascadeCount = 4u;
		static const uint32_t MaxCascadeCount = 6u;

		float cascadeSplitLambda = 0.8f;
		
		bool showCascades = false;
		bool softShadows = true;

		float depthBiasConstant = 0.0f;
		float depthBiasSlope = 0.4f;
	};

	struct ShadowRenderData {
		Texture depthTexture;
		std::array<Texture, ShadowPreferences::MaxCascadeCount> depthTextureLayers;
		std::array<ResourceID, ShadowPreferences::MaxCascadeCount> depthFramebuffers;
		LightType lightType;
		bool isInitialized = false;

		ShadowRenderData() {
			depthFramebuffers.fill(NULL_RESOURCE);
		}
	};

	struct alignas(16) ShadowShaderData {
		glm::mat4 lightMat[ShadowPreferences::MaxCascadeCount];
		uint32_t mapIndex;
	};

	class ShadowRenderLayer : public IRenderLayer<ShadowRenderData, ShadowPreferences> {
	private:
		struct ShadowPreferencesShaderData {
			uint32_t shadowsEnabled = true;
			uint32_t cascadeCount;
			uint32_t softShadows;
			uint32_t showDebugCascades;
			alignas(16) float cascadeSplits[6];
		};

		sa::Format m_depthFormat;

		ResourceID m_depthRenderProgram = NULL_RESOURCE;
		
		std::array<Texture, MAX_SHADOW_TEXTURE_COUNT> m_shadowTextures;
		uint32_t m_shadowTextureCount;

		std::array<Texture, MAX_SHADOW_TEXTURE_COUNT> m_shadowCubeTextures;
		uint32_t m_shadowCubeTextureCount;


		DynamicBuffer m_shadowShaderDataBuffer;
		
		ResourceID m_shadowSampler = NULL_RESOURCE;

		Buffer m_preferencesBuffer;

		std::array<float, ShadowPreferences::MaxCascadeCount> m_cascadeSplitsNormalized;
		std::array<float, ShadowPreferences::MaxCascadeCount> m_cascadeSplits;

		struct MaterialShadowPipeline {
			PipelineLayout pipelineLayout;
			ResourceID pipeline;
			bool isInitialized = false;
		};

		std::unordered_map<UUID, MaterialShadowPipeline> m_materialShaderPipelines;

		void createSampler();

		void initMaterialShadowPipeline(MaterialShader* pMaterialShader, MaterialShadowPipeline& data);

		void cleanupRenderData(ShadowRenderData& data);
		void initializeRenderData(ShadowRenderData& data, LightType lightType);

		void renderMaterialCollection(RenderContext& context, MaterialShaderCollection& collection, ShadowData& data, const ShadowRenderData& renderData, uint32_t layer);

		// Directional lights
		void updateCascadeSplits(float near, float far);
		void calculateCascadeMatrices(const SceneCamera& sceneCamera, ShadowData& data);
		void renderCascadedShadowMaps(RenderContext& context, const SceneCamera& sceneCamera, ShadowData& data, const ShadowRenderData& renderData, SceneCollection& sceneCollection);

		// Point lights
		void renderCubeMapShadows(RenderContext& context, ShadowData& data, const ShadowRenderData& renderData, SceneCollection& sceneCollection);

		// Spot lights
		void renderSingleDirectedShadow(RenderContext& context, ShadowData& data, const ShadowRenderData& renderData, SceneCollection& sceneCollection);


		void renderShadowMap(RenderContext& context, const SceneCamera& sceneCamera, ShadowData& data, const ShadowRenderData& renderData, SceneCollection& sceneCollection);
		
		void renderShadowMap(
			RenderContext& context,
			const SceneCamera& sceneCamera,
			ShadowData& data,
			ShadowRenderData& renderData,
			SceneCollection& sceneCollection,
			uint32_t& shadowCount,
			std::array<Texture, MAX_SHADOW_TEXTURE_COUNT>& shadowTextures,
			uint32_t layerCount,
			uint32_t index);

	public:

		virtual void init() override;
		virtual void cleanup() override;

		virtual void onRenderTargetResize(UUID renderTargetID, Extent oldExtent, Extent newExtent) override;
		virtual void onPreferencesUpdated() override;

		virtual bool preRender(RenderContext& context, SceneCollection& sceneCollection) override;
		virtual bool render(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;
		virtual bool postRender(RenderContext& context, SceneCamera* pCamera, RenderTarget* pRenderTarget, SceneCollection& sceneCollection) override;

		const Buffer& getShadowDataBuffer() const;
		
		const std::array<Texture, MAX_SHADOW_TEXTURE_COUNT>& getShadowTextures() const;
		const uint32_t getShadowTextureCount() const;

		const std::array<Texture, MAX_SHADOW_TEXTURE_COUNT>& getShadowCubeTextures() const;
		const uint32_t getShadowCubeTextureCount() const;

		const ResourceID getShadowSampler() const;
		const Buffer& getPreferencesBuffer() const;

	};

}