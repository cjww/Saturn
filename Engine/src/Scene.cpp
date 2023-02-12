#include "pch.h"
#include "Scene.h"
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
	}


	void Scene::updatePhysics(float dt) {
		// Physics
		m_reg.view<comp::RigidBody, comp::Transform>().each([&](const comp::RigidBody& rb, const comp::Transform& transform) {
			rb.pActor->setGlobalPose(transform, false);
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

	Scene::Scene(const AssetHeader& header) : IAsset(header) {
		m_pPhysicsScene = nullptr;
		registerComponentCallBacks();
	}

	Scene::~Scene() {
		if(m_pPhysicsScene)
			m_pPhysicsScene->release();
	}

	void Scene::reg() {

		auto type = LuaAccessable::registerType<Scene>();
		type["findEntitiesByName"] = [](Scene& self, const std::string& name) {
			std::vector<Entity> entities;
			self.m_reg.each([&](entt::entity e) {
				if (self.m_reg.get<comp::Name>(e).name == name) {
					entities.emplace_back(&self, e);
				}
			});
			return sol::as_table(entities);
		};

		type["createEntity"] = &Scene::createEntity;

	}

	bool Scene::create(const std::string& name) {
		m_name = name;
		m_isLoaded = true;
		
		return true;
	}

	bool Scene::load() {
		return dispatchLoad([&](std::ifstream& file) {
			m_pPhysicsScene = PhysicsSystem::get().createScene();

			simdjson::padded_string jsonStr(m_header.size);
			file.read(jsonStr.data(), jsonStr.length());

			simdjson::ondemand::parser parser;
			auto doc = parser.iterate(jsonStr);
			if (doc.error() != simdjson::error_code::SUCCESS) {
				SA_DEBUG_LOG_ERROR("Json error: ", simdjson::error_message(doc.error()));
				return false;
			}
			deserialize(&doc);

			return true;
		});
	}
	
	bool Scene::write() {
		return dispatchWrite([&](std::ofstream& file) {
			Serializer s;
			serialize(s);
			std::string jsonStr = s.dump();

			m_header.size = jsonStr.size();
			file.seekp((size_t)file.tellp() - sizeof(m_header));
			writeHeader(m_header, file);
			
			file << s.dump();

			return true;
		});
	}

	bool Scene::unload() {
		m_reg.clear();
		m_reg.shrink_to_fit();
		m_scriptManager.freeMemory();
		m_hierarchy.freeMemory();

		if (m_pPhysicsScene) {
			m_pPhysicsScene->release();
			m_pPhysicsScene = nullptr;
		}

		m_isLoaded = false;
		return true;
	}

	void Scene::onRuntimeStart() {
		m_scriptManager.broadcast("onStart");
	}

	void Scene::onRuntimeStop()	{
		m_scriptManager.broadcast("onStop");
	}

	void Scene::runtimeUpdate(float dt) {
		SA_PROFILE_FUNCTION();

		updatePhysics(dt);
		
		// Scripts
		m_scriptManager.broadcast("onUpdate", dt);
		
		updateChildPositions();
		updateCameraPositions();
	}

	void Scene::inEditorUpdate(float dt) {
		updateChildPositions();
		updateCameraPositions();
	}

	void Scene::serialize(Serializer& s) {
		s.beginObject();
		s.value("name", m_name.c_str());

		s.beginArray("entities");

		forEach([&](Entity entity) {
			entity.serialize(s);
		});

		s.endArray();

		s.endObject();
	}

	void Scene::deserialize(void* pDoc) {
		clearEntities();
		using namespace simdjson;
		ondemand::document& doc = *(ondemand::document*)pDoc;
		
		m_name = doc["name"].get_string().value();
		ondemand::array entities = doc["entities"];
		
		m_reg.reserve(entities.count_elements());
		for (auto e : entities) {
			if (e.error()) {
				SA_DEBUG_LOG_WARNING("Failed to get entity from file");
				continue;
			}
			ondemand::object obj = e.value_unsafe().get_object();
			uint32_t id = obj["id"].get_uint64();
			
			Entity entity(this, m_reg.create((entt::entity)id));
			entity.deserialize(&obj);
		}
	}

	void Scene::setScene(const std::string& name) {
		publish<scene_event::SceneRequest>(name);
	}

	Entity Scene::createEntity(const std::string& name) {
		Entity e(this, m_reg.create());
		e.addComponent<comp::Name>(name);
		publish<scene_event::EntityCreated>(e);
		return e;
	}

	void Scene::destroyEntity(const Entity& entity) {
		publish<scene_event::EntityDestroyed>(entity);
		m_scriptManager.clearEntity(entity);
		m_hierarchy.destroy(entity);
		m_reg.destroy(entity);
	}

	size_t Scene::getEntityCount() const {
		return m_reg.alive();
	}

	EntityScript* Scene::addScript(const Entity& entity, const std::filesystem::path& path) {
		return m_scriptManager.addScript(entity, path);
	}

	void Scene::clearEntities() {
		m_reg.clear();
		m_scriptManager.clearAll();
		m_hierarchy.clear();
	}


	void Scene::removeScript(const Entity& entity, const std::string& name) {
		m_scriptManager.removeScript(entity, name);
	}

	EntityScript* Scene::getScript(const Entity& entity, const std::string& name) {
		return m_scriptManager.getScript(entity, name);
	}


	std::vector<EntityScript> Scene::getAssignedScripts(const Entity& entity) const {
		return m_scriptManager.getEntityScripts(entity);
	}

	EntityHierarchy& Scene::getHierarchy() {
		return m_hierarchy;
	}

	const std::string& Scene::getName() const {
		return m_name;
	}

	void Scene::forEachComponentType(std::function<void(ComponentType)> function) {
		m_reg.visit([&](const entt::type_info& info) {
			function(entt::resolve(info));
		});
	}

}