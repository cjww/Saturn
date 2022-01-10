#include "Scene.h"

namespace sa {
	Scene::~Scene() {
		for (auto& cam : m_cameras) {
			delete cam;
		}
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
		publish<AddCamera>(camera);
	}

	void Scene::removeActiveCamera(Camera* camera) {
		publish<RemoveCamera>(camera);
	}

	void Scene::setScene(const std::string& name) {
		publish<SceneSet>(name);
	}

	Entity Scene::createEntity(const std::string name) {
		return Entity(&m_reg, name);
	}

	entt::registry& Scene::getRegistry() {
		return m_reg;
	}
}