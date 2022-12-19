#pragma once
#include "Graphics/IRenderTechnique.h"

#include "AssetManager.h"

#include "Resources/DynamicTexture.hpp"
#include "Resources/DynamicBuffer.hpp"

#define TILE_SIZE 16U
#define MAX_LIGHTS_PER_TILE 1024


namespace sa {

	struct alignas(16) ObjectData {
		Matrix4x4 worldMat;
	};

	class ForwardPlus : public IRenderTechnique {
	private:

		// Color pass
		Texture2D m_colorTexture;
		Texture2D m_depthTexture;

		ResourceID m_colorRenderProgram = NULL_RESOURCE;
		ResourceID m_colorFramebuffer = NULL_RESOURCE;
		ResourceID m_colorPipeline = NULL_RESOURCE;

		ResourceID m_sceneDescriptorSet = NULL_RESOURCE;
		
		ResourceID m_linearSampler = NULL_RESOURCE;
		ResourceID m_nearestSampler = NULL_RESOURCE;

		ResourceID m_depthPreRenderProgram = NULL_RESOURCE;
		ResourceID m_depthPreFramebuffer = NULL_RESOURCE;
		ResourceID m_depthPrePipeline = NULL_RESOURCE;
		ResourceID m_sceneDepthDescriptorSet = NULL_RESOURCE;

		ResourceID m_lightCullingPipeline = NULL_RESOURCE;
		ResourceID m_lightCullingDescriptorSet = NULL_RESOURCE;

		
		
		Vector2u m_tileCount;
		Buffer m_lightIndexBuffer;

		ResourceID m_debugLightHeatmapRenderProgram = NULL_RESOURCE;
		ResourceID m_debugLightHeatmapPipeline = NULL_RESOURCE;
		ResourceID m_debugLightHeatmapFramebuffer = NULL_RESOURCE;
		Texture2D m_debugLightHeatmap;
		ResourceID m_debugLightHeatmapDescriptorSet = NULL_RESOURCE;
		
		// DrawData
		// used for collecting meshes every frame
		std::vector<ModelData*> m_models;
		std::vector<std::vector<ObjectData>> m_objects;
		std::vector<Texture> m_textures;
		std::vector<Material*> m_materials;
		std::vector<Material::Values> m_materialData;
		std::vector<uint32_t> m_materialIndices;

		std::vector<LightData> m_lights;
		

		// These could expand
		DynamicBuffer m_lightBuffer;
		DynamicBuffer m_vertexBuffer;
		DynamicBuffer m_indexBuffer;
		DynamicBuffer m_indirectIndexedBuffer;
		DynamicBuffer m_objectBuffer;
		DynamicBuffer m_materialBuffer;
		DynamicBuffer m_materialIndicesBuffer;

		void createPreDepthPass(Extent extent);
		void createLightCullingShader();
		void createColorPass(Extent extent);

		void resizeLightIndexBuffer(Extent extent);
		
		void updateDescriptorSets();

		

	public:
		using IRenderTechnique::IRenderTechnique;

		virtual void onWindowResize(Extent extent) override;

		virtual void init(Extent extent);
		virtual void cleanup() override;

		virtual void updateData(RenderContext& context) override;
		virtual bool prepareRender(RenderContext& context, SceneCamera* pCamera) override;
		virtual void render(RenderContext& context, SceneCamera* pCamera, ResourceID framebuffer) override;
		virtual void endRender(RenderContext& context) override;

		virtual ResourceID createColorFramebuffer(const Texture2D& outputTexture) override;

		virtual void updateLights(Scene* pScene) override;
		virtual void collectMeshes(Scene* pScene) override;

		const Texture2D& getLightHeatmap() const;
		void setShowHeatmap(bool value);

	};
}

