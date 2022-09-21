#include "pch.h"
#include "Scene.h"

#include "ECS\Components.h"

namespace sa {

	

	Scene::Scene() {
		
	}

	Scene::~Scene() {
		for (auto& cam : m_cameras) {
			delete cam;
		}
	}

	void Scene::init() {
		m_scriptManager.init(this);
	}

	void Scene::update(float dt) {
		SA_PROFILE_FUNCTION();
		publish<scene_event::UpdatedScene>(dt);
		m_scriptManager.update(dt, this);
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
		destroy(entity);
	}

	void Scene::addScript(const Entity& entity, const std::filesystem::path& path) {
		m_scriptManager.addScript(entity, path);
	}

	void Scene::removeScript(const Entity& entity, const std::string& name) {
		m_scriptManager.removeScript(entity, name);
	}

	std::vector<ScriptManager::EntityScript> Scene::getAssignedScripts(const Entity& entity) const {
		return m_scriptManager.getEntityScripts(entity);
	}

	size_t Scene::getEntityCount() const {
		return size();
	}

}