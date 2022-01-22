#include "Scene.h"

#include "ECS\Components.h"

namespace sa {
	void Scene::onModelConstruct(entt::registry reg, entt::entity e) {
		//reg.get<comp::Model>(e).
	}

	void Scene::onModelDestroy(entt::registry reg, entt::entity e) {

	}

	Scene::Scene() {
		
	}

	Scene::~Scene() {
		for (auto& cam : m_cameras) {
			delete cam;
		}
	}

	void Scene::update(float dt) {

	}

	void Scene::render() {

	}

	Camera* Scene::newCamera() {
		m_cameras.push_back(new Camera());
		return m_cameras.back();
	}

	Camera* Scene::newCamera(const RenderWindow* pWindow) {
		m_cameras.push_back(new Camera(pWindow));
		return m_cameras.back();
	}


	void Scene::addActiveCamera(Camera* camera) {
		size_t s = m_activeCameras.size();
		m_activeCameras.insert(camera);
		if(s != m_activeCameras.size())
			publish<AddCamera>(camera);
	}

	void Scene::removeActiveCamera(Camera* camera) {
		size_t s = m_activeCameras.size();
		m_activeCameras.erase(camera);
		if (s != m_activeCameras.size())
			publish<RemoveCamera>(camera);
	}

	std::set<Camera*> Scene::getActiveCameras() const {
		return m_activeCameras;
	}

	void Scene::setScene(const std::string& name) {
		publish<SceneSet>(name);
	}

	Entity Scene::createEntity(const std::string name) {
		return Entity(&m_reg, m_reg.create());
	}

	entt::registry& Scene::getRegistry() {
		return m_reg;
	}
}