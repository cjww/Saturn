#include "pch.h"
#include "Scene.h"
#include "ECS/Components.h"

namespace sa {
	void Scene::updatePhysics(float dt) {
		// Physics
		view<comp::RigidBody, comp::Transform>().each([&](const comp::RigidBody& rb, const comp::Transform& transform) {
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
		view<comp::Camera, comp::Transform>().each([](comp::Camera& camera, comp::Transform& transform) {
			camera.camera.setPosition(transform.position);
			glm::vec3 forward = transform.rotation * glm::vec3(0, 0, 1);
			camera.camera.lookAt(transform.position + forward);
		});
	}

	Scene::Scene(const std::string& name)
		: m_name(name)
		, m_pPhysicsScene(PhysicsSystem::get().createScene())
	{


	
	}


	Scene::~Scene() {
		clearEntities();
		m_pPhysicsScene->release();
	}

	void Scene::reg() {
		auto type = LuaAccessable::registerType<Scene>();
		type["findEntitiesByName"] = [](Scene& self, const std::string& name) {
			std::vector<Entity> entities;
			self.each([&](entt::entity e) {
				if (self.get<comp::Name>(e).name == name) {
					entities.emplace_back(&self, e);
				}
			});
			return sol::as_table(entities);
		};

		type["createEntity"] = &Scene::createEntity;

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
		
		reserve(entities.count_elements());
		for (auto e : entities) {
			if (e.error()) {
				SA_DEBUG_LOG_WARNING("Failed to get entity from file");
				continue;
			}
			ondemand::object obj = e.value_unsafe().get_object();
			uint32_t id = obj["id"].get_uint64();
			
			Entity entity(this, create((entt::entity)id));
			entity.deserialize(&obj);
		}
	}

	void Scene::setScene(const std::string& name) {
		publish<scene_event::SceneRequest>(name);
	}

	Entity Scene::createEntity(const std::string& name) {
		Entity e(this, create());
		e.addComponent<comp::Name>(name);
		publish<scene_event::EntityCreated>(e);
		return e;
	}

	void Scene::destroyEntity(const Entity& entity) {
		publish<scene_event::EntityDestroyed>(entity);
		m_scriptManager.clearEntity(entity);
		m_hierarchy.destroy(entity);
		destroy(entity);
	}

	size_t Scene::getEntityCount() const {
		return alive();
	}

	std::optional<EntityScript> Scene::addScript(const Entity& entity, const std::filesystem::path& path) {
		std::optional<EntityScript> scriptOpt = m_scriptManager.addScript(entity, path);
		if (!scriptOpt.has_value())
			return scriptOpt;

		EntityScript& script = scriptOpt.value();
		script.env["scene"] = this;
		script.env["entity"] = entity;
		
		return scriptOpt;
	}

	void Scene::clearEntities() {
		((entt::registry*)this)->clear();
		m_scriptManager.clearAll();
		m_hierarchy.clear();
	}


	void Scene::removeScript(const Entity& entity, const std::string& name) {
		m_scriptManager.removeScript(entity, name);
	}

	std::optional<EntityScript> Scene::getScript(const Entity& entity, const std::string& name) const {
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
		visit([&](const entt::type_info& info) {
			function(entt::resolve(info));
		});
	}

}