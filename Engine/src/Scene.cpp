#include "pch.h"
#include "Scene.h"

#include "ECS\Components.h"

#include "PhysicsSystem.h"
#include <PxScene.h>
#include <PxRigidBody.h>

namespace sa {

	void Scene::onRigidBodyConstruct(entt::registry& reg, entt::entity e) {
		comp::RigidBody& rb = reg.get<comp::RigidBody>(e);
		comp::Transform& transform = reg.get_or_emplace<comp::Transform>(e);
		rb.pActor = PhysicsSystem::get().createRigidBody(rb.isStatic, transform);
		m_pPhysicsScene->addActor(*rb.pActor);
	}

	void Scene::onRigidBodyDestroy(entt::registry& reg, entt::entity e) {
		if (reg.any_of<comp::SphereCollider>(e)) {
			reg.erase<comp::SphereCollider>(e);
		}
		comp::RigidBody& rb = reg.get<comp::RigidBody>(e);
		m_pPhysicsScene->removeActor(*rb.pActor);
		rb.pActor->release();
	}

	void Scene::onSphereColliderConstruct(entt::registry& reg, entt::entity e) {
		comp::SphereCollider& sc = reg.get<comp::SphereCollider>(e);
		comp::RigidBody& rb = reg.get_or_emplace<comp::RigidBody>(e, true);
		sc.pShape = PhysicsSystem::get().createSphere(sc.radius);
		rb.pActor->attachShape(*sc.pShape);
		
	}

	void Scene::onSphereColliderDestroy(entt::registry& reg, entt::entity e) {
		comp::SphereCollider& sc = reg.get<comp::SphereCollider>(e);
		comp::RigidBody& rb = reg.get<comp::RigidBody>(e);
		rb.pActor->detachShape(*sc.pShape);
		sc.pShape->release();
	}

	void Scene::onBoxColliderConstruct(entt::registry& reg, entt::entity e) {
		comp::BoxCollider& bc = reg.get<comp::BoxCollider>(e);
		comp::RigidBody& rb = reg.get_or_emplace<comp::RigidBody>(e, true);
		bc.pShape = PhysicsSystem::get().createBox(bc.halfLengths);
		rb.pActor->attachShape(*bc.pShape);

	}

	void Scene::onBoxColliderDestroy(entt::registry& reg, entt::entity e) {
		comp::BoxCollider& bc = reg.get<comp::BoxCollider>(e);
		comp::RigidBody& rb = reg.get<comp::RigidBody>(e);
		rb.pActor->detachShape(*bc.pShape);
		bc.pShape->release();
	}

	Scene::Scene(const std::string& name)
		: m_isLoaded(false)
		, m_name(name)
		, m_pPhysicsScene(PhysicsSystem::get().createScene())
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

		view<comp::RigidBody, comp::Transform>().each([&](const comp::RigidBody& rb, const comp::Transform& transform) {
			rb.pActor->setGlobalPose(transform, false);
		});

		m_pPhysicsScene->simulate(dt);
		m_pPhysicsScene->fetchResults(true);

		view<comp::RigidBody, comp::Transform>().each([&](const comp::RigidBody& rb, comp::Transform& transform) {
			transform = rb.pActor->getGlobalPose();
		});

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