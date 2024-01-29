#pragma once

#include "Material.h"
#include "Assets/ModelAsset.h"
#include "ECS/Events.h"
#include "ECS/Components/Model.h"
#include "ECS\Components\Light.h"

namespace sa {
	class Scene;

	struct alignas(16) ObjectData {
		glm::mat4 worldMat;
		bool operator==(const ObjectData&) const = default;
	};
	
	class MaterialShaderCollection {
	private:
		friend class SceneCollection;

		
		std::vector<ModelAsset*> m_models;
		std::vector<std::vector<uint32_t>> m_meshes;

		std::vector<std::vector<Entity>> m_objects;
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


		UUID m_materialShaderID;

		ResourceID m_sceneDescriptorSetColorPass = NULL_RESOURCE;
		ResourceID m_sceneDescriptorSetDepthPass = NULL_RESOURCE;

		ResourceID m_colorPipeline = NULL_RESOURCE;
		ResourceID m_depthPipeline = NULL_RESOURCE;
		Extent m_currentExtent;


	public:

		MaterialShaderCollection(MaterialShader* pMaterialShader);

		void addMesh(ModelAsset* pModelAsset, uint32_t meshIndex, const Entity& entity);
		void removeMesh(const ModelAsset* pModelAsset, uint32_t meshIndex, const Entity& entity);

		void clear();
		void swap();

		bool readyDescriptorSets();
		void recreatePipelines(ResourceID colorRenderProgram, ResourceID depthRenderProgram, Extent extent);

		void bindColorPipeline(RenderContext& context);
		void bindDepthPipeline(RenderContext& context);


		const Buffer& getVertexBuffer() const;
		const Buffer& getIndexBuffer() const;
		const Buffer& getDrawCommandBuffer() const;
		const Buffer& getObjectBuffer() const;
		const Buffer& getMaterialBuffer() const;
		const Buffer& getMaterialIndicesBuffer() const;

		const std::vector<Texture>& getTextures() const;

		ResourceID getSceneDescriptorSetColorPass() const;
		ResourceID getSceneDescriptorSetDepthPass() const;

		MaterialShader* getMaterialShader() const;

	};

	class SceneCollection {
	public:
		enum class CollectionMode {
			REACTIVE,
			CONTINUOUS
		};
	private:
		// DrawData
		// used for collecting meshes every frame
		std::vector<LightData> m_lights;

		// These could expand
		DynamicBuffer m_lightBuffer;

		std::vector<MaterialShaderCollection> m_materialShaderCollections;
		
		MaterialShaderCollection& getMaterialShaderCollection(MaterialShader* pMaterialShader);

		std::vector<entt::connection> m_connections;

		CollectionMode m_mode;
		std::unordered_map<Entity, UUID> m_entityModels;
		std::unordered_set<Entity> m_entitiesToAdd;

		std::unordered_map<Entity, LightData> m_entityLights;

		void addQueuedEntities();


		void onModelConstruct(const scene_event::ComponentCreated<comp::Model>& e);
		void onModelUpdate(const scene_event::ComponentUpdated<comp::Model>& e);
		void onModelDestroy(const scene_event::ComponentDestroyed<comp::Model>& e);

		void onLightConstruct(const scene_event::ComponentCreated<comp::Light>& e);
		void onLightUpdate(const scene_event::ComponentUpdated<comp::Light>& e);
		void onLightDestroy(const scene_event::ComponentDestroyed<comp::Light>& e);


	public:

		SceneCollection(CollectionMode mode);
		virtual ~SceneCollection();

		void clear();

		void collect(Scene* pScene);
		void listen(Scene* pScene);
		void stopListen();


		void setMode(CollectionMode mode);
		CollectionMode getMode() const;


		void addObject(const Entity& entity, ModelAsset* pModel);
		void addLight(const LightData& light);

		void removeObject(const Entity& entity, ModelAsset* pModel);
		void removeLight(const LightData& light);

		void makeRenderReady();
		void swap();

		const Buffer& getLightBuffer() const;
		
		std::vector<MaterialShaderCollection>::iterator begin();
		std::vector<MaterialShaderCollection>::iterator end();

	};
}