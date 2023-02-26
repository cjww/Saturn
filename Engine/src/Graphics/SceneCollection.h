#pragma once

#include "Material.h"
#include "Assets/ModelAsset.h"



namespace sa {
	struct alignas(16) ObjectData {
		Matrix4x4 worldMat;
	};
	
	class SceneCollection {
		private:
		// DrawData
		// used for collecting meshes every frame
		std::vector<ModelAsset*> m_models;
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
	
		uint32_t m_objectCount = 0;
		uint32_t m_vertexCount = 0;
		uint32_t m_indexCount = 0;
		uint32_t m_uniqueMeshCount = 0;
	public:

		SceneCollection();

		void clear();

		void addObject(glm::mat4 transformation, ModelAsset* pModel);
		void addLight(const LightData& light);

		void makeRenderReady();
	
		const Buffer& getLightBuffer() const;
		const Buffer& getVertexBuffer() const;
		const Buffer& getIndexBuffer() const;
		const Buffer& getDrawCommandBuffer() const;
		const Buffer& getObjectBuffer() const;
		const Buffer& getMaterialBuffer() const;
		const Buffer& getMaterialIndicesBuffer() const;
		
		const std::vector<Texture>& getTextures() const;
	};
}