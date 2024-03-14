#pragma once

#include "SceneCamera.h"
#include "ECS/Entity.h"
#include "ECS/Events.h"

#include "ECS\Components.h"

#include "PhysicsSystem.h"

#include <iostream>

#include "Tools\utils.h"
#include "Tools/Profiler.h"


#include "Lua/ScriptManager.h"
#include "EntityHierarchy.h"

#include "Serializable.h"

#include "Assets/Asset.h"


namespace sa {
	class Scene : public entt::dispatcher, public Serializable, public Asset {
	private:
		
		entt::registry m_reg;

		ScriptManager m_scriptManager;

		EntityHierarchy m_hierarchy;
		
		SceneCollection m_dynamicSceneCollection;

		bool m_runtime;

		friend class comp::RigidBody;
		physx::PxScene* m_pPhysicsScene;
	
		friend class Entity;
		void destroyEntity(const Entity& entity);
		EntityScript* addScript(const Entity& entity, const std::filesystem::path& path, const std::unordered_map<std::string, sol::object>& serializedData = {});
		void removeScript(const Entity& entity, const std::string& name);
		EntityScript* getScript(const Entity& entity, const std::string& name) const;

		template<typename T>
		void onComponentConstruct(entt::registry& reg, entt::entity e);
		template<typename T>
		void onComponentUpdate(entt::registry& reg, entt::entity e);
		template<typename T>
		void onComponentDestroy(entt::registry& reg, entt::entity e);

		template<typename T>
		void registerComponentCallBack();
		void registerComponentCallBacks();


		void updatePhysics(float dt);
		void updateChildPositions();
		void updateCameraPositions();
		void updateLightPositions();

	public:
		Scene(const AssetHeader& header);
		virtual ~Scene() override;

		virtual bool onLoad(std::ifstream& file, AssetLoadFlags flags) override; // Asset
		virtual bool onWrite(std::ofstream& file, AssetWriteFlags flags) override; // Asset
		virtual bool onUnload() override; // Asset

		virtual void onRuntimeStart();
		virtual void onRuntimeStop();

		virtual void runtimeUpdate(float dt);
		virtual void inEditorUpdate(float dt);

		void serialize(Serializer& s) override;
		void deserialize(void* pDoc) override;

		Scene* clone(const std::string& name);

		// Event emitter
		void setScene(const std::string& name);

		// Entity
		Entity createEntity(const std::string& name = "Entity", entt::entity idHint = entt::null);
		size_t getEntityCount() const;
		void clearEntities();

		// Scripts
		std::vector<EntityScript*> getAssignedScripts(const Entity& entity);
		void reloadScripts();
		void reloadScript(EntityScript* pScript);

		// Hierarchy
		EntityHierarchy& getHierarchy();

		
		SceneCollection& getDynamicSceneCollection();

		template<typename ...T, typename F, std::enable_if_t<std::is_assignable_v<std::function<void(Entity, T&...)>, F>, bool> = true>
		void forEach(F func);

		template<typename ...T, typename F, std::enable_if_t<std::is_assignable_v<std::function<void(T&...)>, F>, bool> = true>
		void forEach(F func);

		template<typename ...T, typename F, std::enable_if_t<std::is_assignable_v<std::function<void(Entity)>, F>, bool> = true>
		void forEach(F func);

		template<typename F, std::enable_if_t<std::is_assignable_v<std::function<void(Entity)>, F>, bool> = true>
		void forEachEntity(F func);


		void forEachComponentType(std::function<void(ComponentType)> function);


	};
	
	

	template<typename T>
	inline void Scene::onComponentConstruct(entt::registry& reg, entt::entity e) {
		Entity entity(this, e);
		T& comp = reg.get<T>(e);
		comp.onConstruct(&entity);
		trigger<scene_event::ComponentCreated<T>>(scene_event::ComponentCreated<T>{ entity });
	}

	template<typename T>
	inline void Scene::onComponentUpdate(entt::registry& reg, entt::entity e) {
		Entity entity(this, e);
		T& comp = reg.get<T>(e);
		comp.onUpdate(&entity);
		trigger<scene_event::ComponentUpdated<T>>(scene_event::ComponentUpdated<T>{ entity });
	}

	template<typename T>
	inline void Scene::onComponentDestroy(entt::registry& reg, entt::entity e) {
		Entity entity(this, e);
		T& comp = reg.get<T>(e);
		comp.onDestroy(&entity);
		trigger<scene_event::ComponentDestroyed<T>>(scene_event::ComponentDestroyed<T>{ entity });
	}


	template<typename T>
	inline void Scene::registerComponentCallBack() {
		m_reg.on_construct<T>().connect<&Scene::onComponentConstruct<T>>(this);
		m_reg.on_update<T>().connect<&Scene::onComponentUpdate<T>>(this);
		m_reg.on_destroy<T>().connect<&Scene::onComponentDestroy<T>>(this);
	}

	template<typename ...T, typename F, std::enable_if_t<std::is_assignable_v<std::function<void(Entity, T&...)>, F>, bool>>
	inline void Scene::forEach(F func) {
		m_reg.view<T...>().each([&](entt::entity e, T&... comp) {
			Entity entity(this, e);
			func(entity, comp...);
		});
	}

	template<typename ...T, typename F, std::enable_if_t<std::is_assignable_v<std::function<void(T&...)>, F>, bool>>
	inline void Scene::forEach(F func) {
		m_reg.view<T...>().each(func);
	}

	template<typename ...T, typename F, std::enable_if_t<std::is_assignable_v<std::function<void(Entity)>, F>, bool>>
	inline void Scene::forEach(F func) {
		m_reg.view<T...>().each([&](entt::entity e, T&... comp) {
			Entity entity(this, e);
			func(entity);
		});
	}

	template<typename F, std::enable_if_t<std::is_assignable_v<std::function<void(Entity)>, F>, bool>>
	inline void Scene::forEachEntity(F func) {
		for (auto [e] : m_reg.storage<entt::entity>().each()) {
			Entity entity(this, e);
			func(entity);
		}
	}

}
