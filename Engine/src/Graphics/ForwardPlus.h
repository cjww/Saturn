#pragma once
#include "Graphics/IRenderTechnique.h"

#define MAX_MATERIAL_COUNT 4096
#define MAX_LIGHT_COUNT 4096

namespace sa {

	struct alignas(16) ObjectData {
		Matrix4x4 worldMat;
	};
	
	struct MaterialBuffer {
		std::array<Material::Values, MAX_MATERIAL_COUNT> materials;
		std::array<glm::uvec4, 2048> meshToMaterialIndex;
	};

	struct LightBuffer {
		uint32_t count;
		alignas(16) std::array<LightData, MAX_LIGHT_COUNT> lights;
	};

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
		
		ResourceID m_linearSampler = NULL_RESOURCE;
		ResourceID m_nearestSampler = NULL_RESOURCE;

		ResourceID m_depthPreRenderProgram = NULL_RESOURCE;
		ResourceID m_depthPreFramebuffer = NULL_RESOURCE;
		ResourceID m_depthPrePipeline = NULL_RESOURCE;
		ResourceID m_sceneDepthDescriptorSet = NULL_RESOURCE;

		ResourceID m_lightCullingPipeline;
		ResourceID m_lightCullingDescriptorSet;

		
		const uint32_t TILE_SIZE = 16U;
		const uint32_t MAX_LIGHTS_PER_TILE = 128;
		Vector2u m_tileCount;
		Buffer m_lightIndexBuffer;

		ResourceID m_debugLightHeatmapRenderProgram;
		ResourceID m_debugLightHeatmapPipeline;
		ResourceID m_debugLightHeatmapFramebuffer;
		Texture2D m_debugLightHeatmap;
		ResourceID m_debugLightHeatmapDescriptorSet;
		bool m_showHeatmapOverlay;

		// used for collecting meshes every frame
		std::vector<ModelData*> m_models;
		std::vector<std::vector<ObjectData>> m_objects;
		std::vector<Material*> m_materials;
		std::vector<Texture> m_textures;
		
		std::vector<LightData> m_lights;

		// These could expand
		DynamicBuffer m_lightBuffer;
		DynamicBuffer m_vertexBuffer;
		DynamicBuffer m_indexBuffer;
		DynamicBuffer m_indirectIndexedBuffer;
		DynamicBuffer m_objectBuffer;
		DynamicBuffer m_materialBuffer;

		void createTextures(Extent extent);
		void createRenderPasses();
		void createFramebuffers(Extent extent);
		void createPipelines(Extent extent);

		void collectMeshes(Scene* pScene);
		

	public:

		virtual void init(sa::RenderWindow* pWindow, bool setupImGui) override;
		virtual void cleanup() override;

		virtual void beginFrameImGUI() override;

		virtual void draw(Scene* pScene) override;

		virtual const Texture& getOutputTexture() const override;

		virtual void updateLights(Scene* pScene) override;


		const Texture2D& getLightHeatmap() const;
		void setShowHeatmap(bool value);

	};
}

