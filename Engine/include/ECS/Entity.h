#pragma once

#include "ComponentType.h"
#include "MetaComponent.h"
#include "ComponentBase.h"
#include "Lua/LuaAccessable.h"

#include "Serializable.h"
#include "Lua/EntityScript.h"

#include <Tools/Logger.hpp>

namespace sa {
	
	class Scene;

	class Entity : public Serializable {
	private:
		Scene* m_pScene;
		entt::registry* m_pRegistry;
		entt::entity m_entity;
		
	public:
		static void reg();
		static sol::usertype<Entity>& getType();

		Entity(Scene* pScene, entt::entity entity);
		
		Entity(const Entity&) = default;
		Entity();
		virtual ~Entity() = default;

		Entity& operator=(const Entity&) = default;
		Entity& operator=(Entity&&) = default;

		virtual void serialize(Serializer& s) override;
		virtual void deserialize(void* pDoc) override;

		Scene* getScene() const;

		template<typename T>
		T* getComponent() const;
		MetaComponent getComponent(ComponentType type) const;
		MetaComponent getComponent(const std::string& name) const;

		template<typename ...Components>
		bool hasComponents() const;

		bool hasComponent(ComponentType type) const;
		bool hasComponent(const std::string& name) const;

		template<typename T, typename ...Args>
		T* addComponent(Args&& ...args);

		MetaComponent addComponent(ComponentType type);
		MetaComponent addComponent(const std::string& name);

		template<typename T>
		void removeComponent();

		void removeComponent(ComponentType type);
		void removeComponent(const std::string& name);

		template<typename ...Components>
		void updateComponents();

		void updateComponent(ComponentType type);
		void updateComponent(const std::string& name);


		EntityScript* addScript(const std::filesystem::path& path, const EntityScript* inheritSerializedData = nullptr);
		void removeScript(const std::string& name);
		EntityScript* getScript(const std::string& name) const;

		void setParent(const Entity& parent);
		void orphan() const;
		const Entity& getParent() const;
		bool hasParent() const;
		bool hasChildren() const;

		void destroy();

		template<typename Comp>
		Comp* copyComponent(const Entity& src);
		MetaComponent copyComponent(ComponentType type, Entity src);

		Entity clone(Scene* pDstScene = nullptr);

		bool isNull() const;

		std::string toString() const;

		operator bool() const {
			return !isNull();
		}

		operator entt::entity() const {
			return m_entity;
		}

		explicit operator uint32_t() const {
			return (uint32_t)m_entity;
		}

		bool operator > (const Entity& other) const {
			return m_entity > other.m_entity;
		}

		bool operator == (const Entity& other) const {
			return m_entity == other.m_entity && m_pScene == other.m_pScene;
		}

		bool operator != (const Entity& other) const {
			return !(m_entity == other.m_entity && m_pScene == other.m_pScene);
		}

	};

	template<typename Comp>
	void registerComponentType();



	// ----------------- Definitions -----------------

	template<typename T>
	inline T* Entity::getComponent() const {
		if (this->isNull()) {
			throw std::runtime_error("[Entity getComponent] Entity is null: " + toString());
		}
		return m_pRegistry->try_get<T>(m_entity);
	}

	template<typename ...Components>
	inline bool Entity::hasComponents() const {
		if (this->isNull()) {
			throw std::runtime_error("[Entity hasComponent] Entity is null: " + toString());
		}
		return m_pRegistry->all_of<Components...>(m_entity);
	}

	template<typename T, typename ...Args>
	inline T* Entity::addComponent(Args&& ... args) {
		if (this->isNull()) {
			throw std::runtime_error("[Entity addComponent] Entity is null: " + toString());
		}
		T* pComponent = nullptr;
		if (m_pRegistry->all_of<T>(m_entity)) {
			pComponent = &m_pRegistry->get<T>(m_entity);
			pComponent->onUpdate(this);
		}
		else {
			pComponent = &m_pRegistry->emplace<T>(m_entity, args...);
		}
		return pComponent;
	}

	template<typename T>
	inline void Entity::removeComponent() {
		if (this->isNull()) {
			throw std::runtime_error("[Entity removeComponent] Entity is null: " + toString());
		}
		m_pRegistry->remove<T>(m_entity);
	}

	template<typename ...Components>
	inline void Entity::updateComponents() {
		if (this->isNull()) {
			throw std::runtime_error("[Entity updateComponent] Entity is null: " + toString());
		}
		if(m_pRegistry->all_of<Components...>(m_entity))
			m_pRegistry->patch<Components...>(m_entity);
	}

	template<typename Comp>
	inline Comp* Entity::copyComponent(const Entity& src) {
		if (this->isNull()) {
			throw std::runtime_error("[Entity copyComponent] Destination Entity is null: " + toString());
		}
		if (src.isNull()) {
			throw std::runtime_error("[Entity copyComponent] Source Entity is null: " + toString());
		}

		Comp& orig = src.m_pRegistry->get<Comp>(src.m_entity);
		Comp& c = m_pRegistry->get_or_emplace<Comp>(m_entity, orig);
		
		c = orig;
		return &c;
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
				.func<&Entity::hasComponents<Comp>>("has"_hs)
				.func<&Entity::getComponent<Comp>>("get"_hs)
				.func<&Entity::addComponent<Comp>>("add"_hs)
				.func<&Entity::removeComponent<Comp>>("remove"_hs)
				.func<&Entity::copyComponent<Comp>>("copy"_hs)
				.func<&Entity::updateComponents<Comp>>("update"_hs)
				;
		
			SA_DEBUG_LOG_INFO("Registered Meta functions for ", getComponentName<Comp>());

			ComponentType::registerComponent<Comp>();
		}
		if constexpr (std::is_base_of_v<sa::LuaAccessable, std::decay_t<Comp>>) {
			
			Comp::reg();
			LuaAccessable::registerComponent<Comp>();

			LuaAccessable::getState()[getComponentName<Comp>()]["Get"] = [](const Entity& entity) {
				return entity.getComponent<Comp>();
			};

			auto& type = Entity::getType();
			std::string name = getComponentName<Comp>();
			std::string varName = utils::toLower(name);
			type[varName] = sol::property(
				[=](const Entity& self) -> sol::lua_value {
					MetaComponent metaComp = self.getComponent(name);
					return LuaAccessable::cast(metaComp);
				},
				[=](Entity& self, sol::lua_value component) {
					if (!component.is<sol::nil_t>()) {
						// Add component
						MetaComponent mc = self.addComponent(name);
						LuaAccessable::copy(mc, component);
						return;
					}
					self.removeComponent(name);
				});

			
			SA_DEBUG_LOG_INFO("Registered Lua property for ", getComponentName<Comp>());

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

/*
// register components with meta types
template<typename Type>
struct entt::type_seq<Type> {
	static entt::id_type value() ENTT_NOEXCEPT {
		static const entt::id_type value = (sa::registerComponentType<Type>(), internal::type_seq::next());
		return value;
	}
};
 */

