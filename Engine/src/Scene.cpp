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

	void Scene::update(float dt) {

		//	for each script file:
		//		load
		//		retrive required component types 
		//		for each entity carrying required component:
		//			set variables and components (entity, transform etc...)
		//			execute update 

		publish<event::UpdatedScene>(dt);
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
			publish<event::AddedCamera>(camera);
	}

	void Scene::removeActiveCamera(Camera* camera) {
		size_t s = m_activeCameras.size();
		m_activeCameras.erase(camera);
		if (s != m_activeCameras.size())
			publish<event::RemovedCamera>(camera);
	}

	std::set<Camera*> Scene::getActiveCameras() const {
		return m_activeCameras;
	}

	void Scene::setScene(const std::string& name) {
		publish<event::SceneSet>(name);
	}

	Entity Scene::createEntity(const std::string& name) {
		Entity e(&m_reg, m_reg.create());
		e.addComponent<comp::Name>(name);
		publish<event::EntityCreated>(e);
		return e;
	}

	void Scene::destroyEntity(const Entity& entity) {
		publish<event::EntityDestroyed>(entity);
		m_reg.destroy(entity);
	}

	size_t Scene::getEntityCount() const {
		return m_reg.size();
	}

	

}