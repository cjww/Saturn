#pragma once

#include "Camera.h"
#include "ECS/Entity.h"
#include "ECS/Events.h"
#include "ECS/Components.h"
#include "PhysicsSystem.h"

#include <iostream>

#include "Tools\utils.h"

#include "ScriptManager.h"
#include "EntityHierarchy.h"

#include "Serializable.h"


namespace sa {
	typedef uint32_t SceneID;

	class Scene : public entt::emitter<Scene>, entt::registry, public Serializable {
	private:
		std::vector<Camera*> m_cameras;
		std::set<Camera*> m_activeCameras;
		/*
		
		using entt::registry::destroy;
		using entt::registry::create;
		*/
		

		ScriptManager m_scriptManager;

		EntityHierarchy m_hierarchy;

		bool m_isLoaded;

		std::string m_name;
		
		friend class comp::RigidBody;
		physx::PxScene* m_pPhysicsScene;
	
		friend class Entity;
		void destroyEntity(const Entity& entity);
		std::optional<EntityScript> addScript(const Entity& entity, const std::filesystem::path& path);
		void removeScript(const Entity& entity, const std::string& name);
		std::optional<EntityScript> getScript(const Entity& entity, const std::string& name) const;

		friend class Engine;
		template<typename T>
		void onComponentConstruct(entt::registry& reg, entt::entity e);
		template<typename T>
		void onComponentUpdate(entt::registry& reg, entt::entity e);
		template<typename T>
		void onComponentDestroy(entt::registry& reg, entt::entity e);


	public:
		using entt::registry::view;
		
		Scene(const std::string& name);

		virtual ~Scene();

		static void reg();

		virtual void load();
		virtual void unload();

		virtual void update(float dt);

		void serialize(Serializer& s) override;
		void deserialize(void* pDoc) override;


		// Camera
		Camera* newCamera();
		Camera* newCamera(const Window* pWindow);

		void addActiveCamera(Camera* camera);
		void removeActiveCamera(Camera* camera);
		std::set<Camera*> getActiveCameras() const;

		// Event emitter
		void setScene(const std::string& name);

		// Entity
		Entity createEntity(const std::string& name = "Entity");
		size_t getEntityCount() const;
		void clearEntities();

		// Scripts
		std::vector<EntityScript> getAssignedScripts(const Entity& entity) const;

		// Hierarchy
		EntityHierarchy& getHierarchy();

		const std::string& getName() const;

		template<typename ...T, typename F>
		void forEach(F func);

		template<typename F>
		void forEach(const std::vector<ComponentType>& components, F func);

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

	template<typename ...T, typename F>
	inline void Scene::forEach(F func) {
		static_assert(
			std::is_assignable_v<std::function<void(Entity, T&...)>, F> ||
			std::is_assignable_v<std::function<void(T&...)>, F> ||
			std::is_assignable_v<std::function<void(Entity)>, F> &&
			"Not a valid function signature");
		if constexpr (std::is_assignable_v<std::function<void(Entity)>, F>) {
			each([&](const entt::entity e) {
				Entity entity(this, e);
				func(entity);
			});
		}
		else if constexpr (std::is_assignable_v<std::function<void(Entity, T&...)>, F>) {
			view<T...>().each([&](entt::entity e, T&... comp) {
				Entity entity(this, e);
				func(entity, comp...);
			});
		}
		else if constexpr (std::is_assignable_v<std::function<void(T&...)>, F>) {
			view<T...>().each(func);
		}

	}


	template<typename F>
	inline void Scene::forEach(const std::vector<ComponentType>& components, F func) {
		using namespace entt::literals;
		
		std::vector<entt::id_type> types(components.size());
		for (size_t i = 0; i < types.size(); i++) {
			types[i] = components[i].getTypeId();
		}

		runtime_view(std::cbegin(types), std::cend(types)).each([&](entt::entity e) {
			Entity entity(this, e);
			func(entity);
		});
	}

}
