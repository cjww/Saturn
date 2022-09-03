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


		// used for collecting meshes every frame
		std::vector<ModelData*> m_models;
		std::vector<std::vector<ObjectData>> m_objects;
		std::vector<Material*> m_materials;
		std::vector<Texture> m_textures;
		
		std::vector<LightData> m_lights;

		Buffer m_sceneUniformBuffer;
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

		virtual Texture getOutputTexture() const override;

		virtual void updateLights(Scene* pScene) override;


	};
}

