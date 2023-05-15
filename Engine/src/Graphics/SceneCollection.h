#pragma once

#include "Material.h"
#include "Assets/ModelAsset.h"
#include "ECS\Components\Light.h"

namespace sa {
	class Scene;

	struct alignas(16) ObjectData {
		glm::mat4 worldMat;
	};
	
	class MaterialShaderCollection {
	private:
		friend class SceneCollection;

		MaterialShader* m_pMaterialShader;

		std::vector<ModelAsset*> m_models;
		std::vector<std::vector<ObjectData>> m_objects;
		std::vector<Texture> m_textures;
		std::vector<Material*> m_materials;
		std::vector<Material::Values> m_materialData;
		std::vector<uint32_t> m_materialIndices;


		// These could expand
		DynamicBuffer m_vertexBuffer;
		DynamicBuffer m_indexBuffer;
		DynamicBuffer m_indirectIndexedBuffer;
		DynamicBuffer m_objectBuffer;
		DynamicBuffer m_materialBuffer;
		DynamicBuffer m_materialIndicesBuffer;

		uint32_t m_objectCount = 0;
		uint32_t m_vertexCount = 0;
		uint32_t m_indexCount = 0;
		uint32_t m_uniqueMeshCount = 0;


	public:
		ResourceID sceneDescriptorSetColorPass = NULL_RESOURCE;
		ResourceID sceneDescriptorSetDepthPass = NULL_RESOURCE;

		MaterialShaderCollection(MaterialShader* pMaterialShader);
		void clear();
		void swap();

		void readyDescriptorSets();

		const Buffer& getVertexBuffer() const;
		const Buffer& getIndexBuffer() const;
		const Buffer& getDrawCommandBuffer() const;
		const Buffer& getObjectBuffer() const;
		const Buffer& getMaterialBuffer() const;
		const Buffer& getMaterialIndicesBuffer() const;

		const std::vector<Texture>& getTextures() const;

		ResourceID getSceneDescriptorSetColorPass() const;
		ResourceID getSceneDescriptorSetDepthPass() const;


	};

	class SceneCollection {
		private:
		// DrawData
		// used for collecting meshes every frame
		std::vector<LightData> m_lights;

		// These could expand
		DynamicBuffer m_lightBuffer;

		std::vector<MaterialShaderCollection> m_materialShaderCollections;
		
		MaterialShaderCollection& getMaterialShaderCollection(MaterialShader* pMaterialShader);

	public:

		SceneCollection();

		void clear();

		void collect(Scene* pScene);

		void addObject(glm::mat4 transformation, ModelAsset* pModel);
		void addLight(const LightData& light);

		void makeRenderReady();
		void swap();

		const Buffer& getLightBuffer() const;
		
		std::vector<MaterialShaderCollection>::iterator begin();
		std::vector<MaterialShaderCollection>::iterator end();

	};
}