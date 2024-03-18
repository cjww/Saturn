#include "pch.h"
#include "Scene.h"

#include <d3d10sdklayers.h>

#include "ECS/Components.h"

namespace sa {
	void Scene::registerComponentCallBacks() {
		registerComponentCallBack<comp::Name>();
		registerComponentCallBack<comp::Transform>();
		registerComponentCallBack<comp::Light>();
		registerComponentCallBack<comp::Model>();
		registerComponentCallBack<comp::RigidBody>();
		registerComponentCallBack<comp::BoxCollider>();
		registerComponentCallBack<comp::SphereCollider>();
		registerComponentCallBack<comp::Camera>();
		registerComponentCallBack<comp::ShadowEmitter>();
	}


	void Scene::updatePhysics(float dt) {
		// Physics
		m_reg.view<comp::RigidBody, comp::Transform>().each([&](comp::RigidBody& rb, const comp::Transform& transform) {
			rb.setGlobalPose(transform);
		});

		m_pPhysicsScene->simulate(dt);
		m_pPhysicsScene->fetchResults(true);

		uint32_t actorCount = -1;
		physx::PxActor** ppActors = m_pPhysicsScene->getActiveActors(actorCount);
		for (uint32_t i = 0U; i < actorCount; i++) {
			physx::PxActor* pActor = ppActors[i];
			if (physx::PxRigidActor* rigidActor = pActor->is<physx::PxRigidActor>()) {
				comp::Transform* transform = ((Entity*)pActor->userData)->getComponent<comp::Transform>();
				*transform = rigidActor->getGlobalPose();
			}
		}
	}

	void Scene::updateChildPositions() {
		m_hierarchy.forEachParent([&](const sa::Entity& parent) {
			m_hierarchy.forEachChild(parent, [](const Entity& child, const Entity& parent) {
				comp::Transform* transform = child.getComponent<comp::Transform>();
				comp::Transform* parentTransform = parent.getComponent<comp::Transform>();
				if (!transform || !parentTransform)
					return;

				transform->position = parentTransform->position + transform->relativePosition;
			});
		});
	}

	void Scene::updateCameraPositions() {
		m_reg.view<comp::Camera, comp::Transform>().each([](comp::Camera& camera, comp::Transform& transform) {
			camera.camera.setPosition(transform.position);
			glm::vec3 forward = transform.rotation * glm::vec3(0, 0, 1);
			camera.camera.lookAt(transform.position + forward);
		});
	}

	void Scene::updateLightPositions() {
		/*
		m_reg.view<comp::Transform, comp::Light>().each([](comp::Transform& transform, comp::Light& light) {
			light.values.position = glm::vec4(transform.position, light.values.position.w);
			light.values.direction = glm::vec4(transform.rotation * glm::vec3(0, 0, 1), light.values.direction.w);
		});
		*/
	}


	Scene::Scene(const AssetHeader& header)
		: Asset(header)
		, m_scriptManager(*this)
		, m_dynamicSceneCollection(sa::SceneCollection::CollectionMode::CONTINUOUS)
	{
		m_runtime = false;
		m_pPhysicsScene = PhysicsSystem::get().createScene();
		registerComponentCallBacks();
	}

	Scene::~Scene() {
		if(m_pPhysicsScene)
			m_pPhysicsScene->release();
	}

	bool Scene::onLoad(std::ifstream& file, AssetLoadFlags flags) {
		simdjson::padded_string jsonStr(getHeader().size);
		file.read(jsonStr.data(), jsonStr.length());

		simdjson::ondemand::parser parser;
		auto doc = parser.iterate(jsonStr);
		if (doc.error() != simdjson::error_code::SUCCESS) {
			throw std::runtime_error("Json error: " + std::string(simdjson::error_message(doc.error())));
		}
		deserialize(&doc);
		
		return true;
	}

	bool Scene::onWrite(std::ofstream& file, AssetWriteFlags flags) {
		Serializer s;
		serialize(s);
		std::string jsonStr = s.dump();

		file << s.dump();

		return true;
	}

	bool Scene::onUnload() {
		m_reg.clear();
		decltype(m_reg) reg;
		m_reg.swap(reg);
		m_scriptManager.clearAll();
		m_scriptManager.freeMemory();
		m_hierarchy.freeMemory();
		return true;
	}

	void Scene::onRuntimeStart() {
		m_runtime = true;
		m_scriptManager.applyChanges();
		trigger<scene_event::SceneStart>();
	}

	void Scene::onRuntimeStop()	{
		if(m_runtime) 
			trigger<scene_event::SceneStop>();
		m_runtime = false;
	}

	void Scene::runtimeUpdate(float dt) {
		SA_PROFILE_FUNCTION();

		updatePhysics(dt);

		m_scriptManager.applyChanges();
		// Scripts
		{
			SA_PROFILE_SCOPE("Update Event");
			trigger<scene_event::SceneUpdate>(scene_event::SceneUpdate{ dt });
		}
		
		updateChildPositions();
		updateCameraPositions();
		updateLightPositions();

	}

	void Scene::inEditorUpdate(float dt) {
		updateChildPositions();
		updateCameraPositions();
		updateLightPositions();

	}

	void Scene::render(RenderContext& context, RenderPipeline& renderPipeline, RenderTarget& mainRenderTarget) {
		m_dynamicSceneCollection.clear();
		m_dynamicSceneCollection.collect(this);
		m_dynamicSceneCollection.makeRenderReady();
		renderPipeline.preRender(context, m_dynamicSceneCollection);

		bool renderedToMainRenderTarget = false;
		forEach<comp::Camera>([&](comp::Camera& camera) {
			RenderTarget* pRenderTarget = camera.getRenderTarget().getAsset();
			if (pRenderTarget) {
				renderPipeline.render(context, &camera.camera, pRenderTarget, m_dynamicSceneCollection);
			}
			else {
				if (!renderedToMainRenderTarget)
					renderPipeline.render(context, &camera.camera, &mainRenderTarget, m_dynamicSceneCollection);
				renderedToMainRenderTarget = true;
			}
		});
	}

	void Scene::serialize(Serializer& s) {
		s.beginObject();
		s.beginArray("entities");

		forEachEntity([&](Entity entity) {
			entity.serialize(s);
		});

		s.endArray();

		s.endObject();
	}

	void Scene::deserialize(void* pDoc) {
		using namespace simdjson;
		ondemand::document& doc = *(ondemand::document*)pDoc;
		ondemand::array entities = doc["entities"];

		
		std::unordered_set<entt::entity> allEntities;
		for(auto [e] : m_reg.storage<entt::entity>().each()) {
			allEntities.insert(e);
		}
		

		m_reg.storage<entt::entity>().reserve(entities.count_elements());
		for (auto e : entities) {
			if (e.error()) {
				SA_DEBUG_LOG_WARNING("Failed to get entity from file");
				continue;
			}
			ondemand::object obj = e.value_unsafe().get_object();
			uint32_t id = (uint32_t)obj["id"].get_uint64();
			entt::entity entityID = (entt::entity)id;
			if (!m_reg.valid(entityID)) {
				entityID = m_reg.create(entityID);
			}
			Entity entity(this, entityID);
			entity.deserialize(&obj);
			allEntities.erase(entityID);
		}
		for(auto& entityId : allEntities) {
			Entity entity(this, entityId);
			entity.destroy();
		}
	}

	Scene* Scene::clone(const std::string& name) {

		auto clone = AssetManager::get().createAsset<Scene>(name, "");
		forEachEntity([&](Entity entity) {
			entity.clone(clone);
		});
		return clone;
	}

	void Scene::setScene(const std::string& name) {
		trigger<scene_event::SceneRequest>(scene_event::SceneRequest{ name });
	}

	Entity Scene::createEntity(const std::string& name, entt::entity idHint) {
		Entity e(this, (idHint == entt::null ? m_reg.create() : m_reg.create(idHint)));
		e.addComponent<comp::Name>(name);
		trigger<scene_event::EntityCreated>(scene_event::EntityCreated{ e });
		return e;
	}

	void Scene::destroyEntity(const Entity& entity) {
		if (entity.isNull())
			throw std::runtime_error("Attempt to destroy null entity: " + entity.toString());
		trigger<scene_event::EntityDestroyed>(scene_event::EntityDestroyed{ entity });
		m_scriptManager.clearEntity(entity);
		m_hierarchy.destroy(entity);
		m_reg.destroy(entity);
	}

	size_t Scene::getEntityCount() const {
		return m_reg.storage<entt::entity>()->in_use();
	}

	EntityScript* Scene::addScript(const Entity& entity, const std::filesystem::path& path, const std::unordered_map<std::string, sol::object>& serializedData) {
		auto pScript = m_scriptManager.addScript(entity, path, serializedData);
		if (pScript) {
			if(m_runtime)
				m_scriptManager.tryCall(pScript->env, scene_event::SceneStart::CallbackName);
		}
		return pScript;
	}

	void Scene::clearEntities() {
		m_reg.clear();
		m_scriptManager.clearAll();
		m_hierarchy.clear();
	}


	void Scene::removeScript(const Entity& entity, const std::string& name) {
		auto pScript = m_scriptManager.getScript(entity, name);
		if(!pScript) 
			return;
		m_scriptManager.removeScript(pScript);
	}

	EntityScript* Scene::getScript(const Entity& entity, const std::string& name) const {
		return m_scriptManager.getScript(entity, name);
	}


	std::vector<EntityScript*> Scene::getAssignedScripts(const Entity& entity) {
		return m_scriptManager.getEntityScripts(entity);
	}

	void Scene::reloadScripts() {
		m_scriptManager.reloadScripts();
	}

	void Scene::reloadScript(EntityScript* pScript) {
		m_scriptManager.reloadScript(pScript);
	}

	EntityHierarchy& Scene::getHierarchy() {
		return m_hierarchy;
	}

	SceneCollection& Scene::getDynamicSceneCollection() {
		return m_dynamicSceneCollection;
	}

	void Scene::forEachComponentType(std::function<void(ComponentType)> function) {
		for(auto&& [id, type] : entt::resolve()) {
			function(type);
		}
	}

}