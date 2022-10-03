#include "pch.h"
#include "Scene.h"

#include "ECS\Components.h"
#include <PxPhysicsAPI.h>


class ErrorCallback : public physx::PxErrorCallback {
	virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override {
		SA_DEBUG_LOG_ERROR("Code:", (uint32_t)code, message, file, line);
	}

};

class AllocatorCallback : public physx::PxAllocatorCallback {
	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line) override {
		return malloc(size);
	}
	virtual void deallocate(void* ptr) override {
		free(ptr);
	}

};

namespace sa {

	Scene::Scene(const std::string& name)
		: m_isLoaded(false)
		, m_name(name)

	{

		static AllocatorCallback s_defaultAllocator;
		static ErrorCallback s_defaultErrorCallback;

		m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_defaultAllocator, s_defaultErrorCallback);
		if (!m_pFoundation) {
			throw std::runtime_error("PxCreateFoundation failed!");
		}

		//m_pPvd = PxCreatePvd(*m_pFoundation);
		//m_pTransport = PxDefaultPvdSocketTransportCreate();
		//m_pPvd->connect()
		m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, physx::PxTolerancesScale());
		if (!m_pPhysics) {
			throw std::runtime_error("PxCreatePhysics failed!");
		}

		physx::PxSceneDesc desc(m_pPhysics->getTolerancesScale());
		desc.gravity = physx::PxVec3(0.0f, -9.82f, 0.0f);
		m_pDefaultCpuDispatcher = physx::PxDefaultCpuDispatcherCreate(4);
		desc.cpuDispatcher = m_pDefaultCpuDispatcher;
		desc.filterShader = physx::PxDefaultSimulationFilterShader;
		/*
		*/

		m_pScene = m_pPhysics->createScene(desc);
		if (!m_pScene)
			throw std::runtime_error("createScene failed!");

		m_testEntity = createEntity("TestPhysics");
		m_testEntity.addComponent<comp::Transform>()->position = {0, 5, 0};
		auto& progress = AssetManager::get().loadModel("resources/models/Suzanne.dae");
		progress.wait();
		m_testEntity.addComponent<comp::Model>()->modelID = progress;

		m_pActor = m_pPhysics->createRigidDynamic(physx::PxTransform(physx::PxVec3(0, 5, 0)));
		m_pScene->addActor(*m_pActor);
	}


	Scene::~Scene() {

		m_pActor->release();
		m_pScene->release();
		m_pDefaultCpuDispatcher->release();

		m_pPhysics->release();
		m_pFoundation->release();

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

		m_pScene->simulate(dt);

		m_pScene->fetchResults(true);

		auto transform = m_pActor->getGlobalPose();
		m_testEntity.getComponent<comp::Transform>()->position = { transform.p.x, transform.p.y, transform.p.z };

		publish<scene_event::UpdatedScene>(dt);
		m_scriptManager.update(dt, this);

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
		return size();
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