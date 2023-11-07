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
	class Scene : public entt::dispatcher, public Serializable, public Asset, LuaAccessable {
	private:
		
		entt::registry m_reg;

		ScriptManager m_scriptManager;

		EntityHierarchy m_hierarchy;
		
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

		static void reg();
		
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

		template<typename ...T, typename F>
		void forEach(F func);

		void forEachComponentType(std::function<void(ComponentType)> function);


	};
	
	

	template<typename T>
	inline void Scene::onComponentConstruct(entt::registry& reg, entt::entity e) {
		Entity entity(this, e);
		reg.get<T>(e).onConstruct(&entity);
	}

	template<typename T>
	inline void Scene::onComponentUpdate(entt::registry& reg, entt::entity e) {
		Entity entity(this, e);
		reg.get<T>(e).onUpdate(&entity);
	}

	template<typename T>
	inline void Scene::onComponentDestroy(entt::registry& reg, entt::entity e) {
		Entity entity(this, e);
		reg.get<T>(e).onDestroy(&entity);
	}

	template<typename T>
	inline void Scene::registerComponentCallBack() {
		m_reg.on_construct<T>().connect<&Scene::onComponentConstruct<T>>(this);
		m_reg.on_update<T>().connect<&Scene::onComponentUpdate<T>>(this);
		m_reg.on_destroy<T>().connect<&Scene::onComponentDestroy<T>>(this);
	}

	template<typename ...T, typename F>
	inline void Scene::forEach(F func) {
		static_assert(
			std::is_assignable_v<std::function<void(Entity, T&...)>, F> ||
			std::is_assignable_v<std::function<void(T&...)>, F> ||
			std::is_assignable_v<std::function<void(Entity)>, F> &&
			"Not a valid function signature");
		if constexpr (std::is_assignable_v<std::function<void(Entity)>, F>) {
			for(auto [e] : m_reg.storage<entt::entity>().each()) {
				Entity entity(this, e);
				func(entity);
			}
		}
		else if constexpr (std::is_assignable_v<std::function<void(Entity, T&...)>, F>) {
			m_reg.view<T...>().each([&](entt::entity e, T&... comp) {
				Entity entity(this, e);
				func(entity, comp...);
			});
		}
		else if constexpr (std::is_assignable_v<std::function<void(T&...)>, F>) {
			m_reg.view<T...>().each(func);
		}

	}
}
