#include "pch.h"
#include "Scene.h"

#include "ECS\Components.h"

#include <simdjson.h>

namespace sa {

	Scene::Scene(const std::string& name)
		: m_isLoaded(false)
		, m_name(name)

	{
	}


	Scene::~Scene() {
		for (auto& cam : m_cameras) {
			delete cam;
		}
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

	void Scene::load() {
		m_scriptManager.init(this);
		m_isLoaded = true;
	}

	void Scene::unload() {
		m_isLoaded = false;
	}

	void Scene::update(float dt) {
		SA_PROFILE_FUNCTION();
		publish<scene_event::UpdatedScene>(dt);
		m_scriptManager.update(dt, this);
		
		/*
		view<comp::Light, comp::Transform>().each([](comp::Light& light, const comp::Transform& transform) {
			light.values.position
		});

		*/
		view<comp::Transform>().each([&](const entt::entity& e, comp::Transform& transform) {
			Entity entity(this, e);
			if (!m_hierarchy.hasChildren(entity)) { 
				return;
			}
			m_hierarchy.forEachChild(entity, [](const Entity& child, const Entity& parent) {
				comp::Transform* transform = child.getComponent<comp::Transform>();
				comp::Transform* parentTransform = parent.getComponent<comp::Transform>();
				if (!transform || !parentTransform)
					return;

				transform->position = parentTransform->position + transform->relativePosition;
			});
			
		});




	}

	void Scene::serialize(Serializer& s) {
		s.beginObject();
		s.value("name", m_name.c_str());
		s.beginArray("entities");
		each([&](entt::entity e) {
			Entity entity(this, e);
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
		for (auto e : doc["entities"]) {
			if (e.error()) {
				SA_DEBUG_LOG_WARNING("Failed to get entity from file");
				continue;
			}
			ondemand::object obj = e.value_unsafe().get_object();
			
			Entity entity = createEntity();
			entity.deserialize(&obj);
		}

	}


	Camera* Scene::newCamera() {
		m_cameras.push_back(new Camera());
		return m_cameras.back();
	}

	Camera* Scene::newCamera(const Window* pWindow) {
		m_cameras.push_back(new Camera(pWindow));
		return m_cameras.back();
	}


	void Scene::addActiveCamera(Camera* camera) {
		size_t s = m_activeCameras.size();
		m_activeCameras.insert(camera);
		if(s != m_activeCameras.size())
			publish<scene_event::AddedCamera>(camera);
	}

	void Scene::removeActiveCamera(Camera* camera) {
		size_t s = m_activeCameras.size();
		m_activeCameras.erase(camera);
		if (s != m_activeCameras.size())
			publish<scene_event::RemovedCamera>(camera);
	}

	std::set<Camera*> Scene::getActiveCameras() const {
		return m_activeCameras;
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
		
		if (!m_isLoaded)
			return scriptOpt;

		m_scriptManager.tryCall(script.env, "init");

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

}