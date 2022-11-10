#pragma once

#include "ComponentType.h"
#include "MetaComponent.h"
#include "ComponentBase.h"

#include "ScriptManager.h"

#include "Serializable.h"

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
		
		Entity(const Entity& other) = default;
		Entity();
		virtual ~Entity() = default;

		virtual void serialize(Serializer& s) override;
		virtual void deserialize(void* pDoc) override;

		Scene* getScene();

		template<typename T>
		T* getComponent() const;
		MetaComponent getComponent(ComponentType type) const;
		MetaComponent getComponent(const std::string& name) const;

		template<typename ...Components>
		bool hasComponent() const;

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
		void update();

		void addScript(const std::filesystem::path& path);
		void removeScript(const std::string& name);
		std::optional<EntityScript> getScript(const std::string& name) const;

		void setParent(const Entity& parent);
		void orphan() const;
		const Entity& getParent() const;
		bool hasParent() const;
		bool hasChildren() const;

		void destroy();

		template<typename Comp>
		Comp* copyComponent(Entity src);
		MetaComponent copyComponent(ComponentType type, Entity src);

		Entity clone();

		bool isNull() const;

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
			throw std::runtime_error("Entity was a null entity");
		}
		return &m_pRegistry->emplace_or_replace<T>(m_entity, args...);
	}

	template<typename T>
	inline void Entity::removeComponent() {
		if (this->isNull()) {
			throw std::runtime_error("Entity was a null entity");
		}
		m_pRegistry->remove<T>(m_entity);
	}

	template<typename ...Components>
	inline void Entity::update() {
		if(m_pRegistry->all_of<Components...>(m_entity))
			m_pRegistry->patch<Components...>(m_entity);
	}

	template<typename Comp>
	inline Comp* Entity::copyComponent(Entity src) {
		Comp& orig = m_pRegistry->get<Comp>(src.m_entity);
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
				.func<&Entity::hasComponent<Comp>>("has"_hs)
				.func<&Entity::getComponent<Comp>, entt::as_ref_t>("get"_hs)
				.func<&Entity::addComponent<Comp>, entt::as_ref_t>("add"_hs)
				.func<&Entity::removeComponent<Comp>>("remove"_hs)
				.func<&Entity::copyComponent<Comp>>("copy"_hs)
				;
		
			SA_DEBUG_LOG_INFO("Registered Meta functions for ", getComponentName<Comp>());

			ComponentType::registerComponent<Comp>();
		}
		if constexpr (std::is_base_of_v<sa::LuaAccessable, std::decay_t<Comp>>) {
			
			LuaAccessable::registerComponent<Comp>();
			Comp::reg();

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

// register components with meta types
template<typename Type>
struct entt::type_seq<Type> {
	static entt::id_type value() ENTT_NOEXCEPT {
		static const entt::id_type value = (sa::registerComponentType<Type>(), internal::type_seq::next());
		return value;
	}
};

