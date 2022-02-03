#pragma once

#include "Camera.h"
#include "ECS/Entity.h"
#include "ECS/Events.h"
#include "ECS/Components.h"

namespace sa {
	typedef uint32_t SceneID;


	class Scene : public entt::emitter<Scene> {
	private:
		std::vector<Camera*> m_cameras;
		std::set<Camera*> m_activeCameras;

		entt::registry m_reg;

	public:
		Scene();
		virtual ~Scene();

		void update(float dt);
		void render();

		Camera* newCamera();
		Camera* newCamera(const RenderWindow* pWindow);

		void addActiveCamera(Camera* camera);
		void removeActiveCamera(Camera* camera);
		std::set<Camera*> getActiveCameras() const;

		void setScene(const std::string& name);

		Entity createEntity(const std::string& name = "Entity");
		void destroyEntity(const Entity& entity);

		size_t getEntityCount() const;

		template<typename ...T, typename F>
		void forEach(F func);

		template<typename F>
		void forEach(const std::vector<ComponentType>& components, F func);

	};
	
	template<typename ...T, typename F>
	inline void Scene::forEach(F func) {
		static_assert(
			std::is_assignable_v<std::function<void(Entity, T&...)>, F> ||
			std::is_assignable_v<std::function<void(T&...)>, F> ||
			std::is_assignable_v<std::function<void(Entity)>, F> &&
			"Not a valid function signature");
		if constexpr (std::is_assignable_v<std::function<void(Entity)>, F>) {
			m_reg.each([&](const entt::entity e) {
				Entity entity(&m_reg, e);
				func(entity);
			});
		}
		else if constexpr (std::is_assignable_v<std::function<void(Entity, T&...)>, F>) {
			m_reg.view<T...>().each([&](entt::entity e, T&... comp) {
				Entity entity(&m_reg, e);
				func(entity, comp...);
			});
		}
		else if constexpr (std::is_assignable_v<std::function<void(T&...)>, F>) {
			m_reg.view<T...>().each(func);
		}

	}

	template<typename F>
	inline void Scene::forEach(const std::vector<ComponentType>& components, F func) {
		using namespace entt::literals;
		
		std::vector<entt::id_type> types(components.size());
		for (size_t i = 0; i < types.size(); i++) {
			types[i] = components[i].getTypeId();
		}
		
		m_reg.runtime_view(std::cbegin(types), std::cend(types)).each([&](entt::entity e) {
			Entity entity(&m_reg, e);
			func(entity);
		});
	}
}
