#pragma once

#include "ComponentType.h"
#include "MetaComponent.h"

#include "ComponentBase.h"


namespace sa {
	
	class Entity {
	private:
		entt::registry* m_pRegistry;
		entt::entity m_entity;
	public:
		Entity(entt::registry* pRegistry, entt::entity entity);
		Entity(const Entity& other) = default;
		Entity();
		virtual ~Entity() = default;

		template<typename T>
		T* getComponent() const;
		MetaComponent getComponent(ComponentType type) const;
		MetaComponent getComponent(const std::string& name) const;

		template<typename ...Components>
		bool hasComponent() const;

		bool hasComponent(ComponentType type) const;
		bool hasComponent(const std::string name) const;

		template<typename T, typename ...Args>
		T* addComponent(Args&& ...args);

		MetaComponent addComponent(ComponentType type);
		MetaComponent addComponent(const std::string& name);

		template<typename T>
		void removeComponent();

		void removeComponent(ComponentType type);
		void removeComponent(const std::string& name);

		bool isNull() const;

		operator bool() const {
			return !isNull();
		}

		operator entt::entity() const {
			return m_entity;
		}

		operator uint32_t() const {
			return (uint32_t)m_entity;
		}

		bool operator > (const Entity& other) const {
			return m_entity > other.m_entity;
		}

		bool operator == (const Entity& other) const {
			return m_entity == other.m_entity && m_pRegistry == other.m_pRegistry;
		}

		bool operator != (const Entity& other) const {
			return !(m_entity == other.m_entity && m_pRegistry == other.m_pRegistry);
		}

	};

	template<typename Comp>
	void registerComponentType();

	void updateEntityType();



	// ----------------- Definitions -----------------

	template<typename T>
	inline T* Entity::getComponent() const {
		if (this->isNull()) {
			return nullptr;
		}
		return m_pRegistry->try_get<T>(m_entity);
	}

	template<typename ...Components>
	inline bool Entity::hasComponent() const {
		return m_pRegistry->all_of<Components...>(m_entity);
	}

	template<typename T, typename ...Args>
	inline T* Entity::addComponent(Args&& ... args) {
		if (this->isNull()) {
			//SA_DEBUG_LOG_ERROR("Entity was a null entity");
			throw std::runtime_error("Entity was a null entity");
		}
		return &m_pRegistry->emplace_or_replace<T>(m_entity, args...);
	}


	template<typename T>
	inline void Entity::removeComponent() {
		if (this->isNull()) {
			//SA_DEBUG_LOG_ERROR("Entity was a null entity");
			throw std::runtime_error("Entity was a null entity");
		}
		m_pRegistry->remove<T>(m_entity);
	}



	template<typename Comp>
	inline void registerComponentType() {
		static bool registered = false;
		if (registered)
			return;

		registered = true;
		if constexpr (std::is_base_of_v<sa::ComponentBase, std::decay_t<Comp>>) {
			
			using namespace entt::literals;
			entt::meta<Comp>()
				.type(entt::hashed_string(getComponentName<Comp>().c_str()))
				.func<&Entity::hasComponent<Comp>>("has"_hs)
				.func<&Entity::getComponent<Comp>, entt::as_ref_t>("get"_hs)
				.func<&Entity::addComponent<Comp>, entt::as_ref_t>("add"_hs)
				.func<&Entity::removeComponent<Comp>>("remove"_hs)
				;
		
			ComponentType::registerComponent<Comp>();
		}
		if constexpr (std::is_base_of_v<sa::LuaAccessable, std::decay_t<Comp>>) {
			
			auto type = LuaAccessable::registerComponent<Comp>();
			Comp::luaReg(type);

			updateEntityType();

		}

	}

	

}

// Hashable
namespace std
{
	template <> struct hash<sa::Entity>
	{
		size_t operator()(const sa::Entity& other)const
		{
			return std::hash<entt::entity>()((entt::entity)other);
		}
	};
}

// register components with meta types
template<typename Type>
struct entt::type_seq<Type> {
	static entt::id_type value() ENTT_NOEXCEPT {
		static const entt::id_type value = (sa::registerComponentType<Type>(), internal::type_seq::next());
		return value;
	}
};

