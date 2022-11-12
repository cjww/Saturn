#pragma once

#include "sol/sol.hpp"
#include "ComponentType.h"
#include "Serializable.h"

namespace sa {
	class Entity;

	class ComponentBase : public Serializable {
	public:
	
		virtual void onConstruct(sa::Entity* e) {};
		virtual void onUpdate(sa::Entity* e) {};
		virtual void onDestroy(sa::Entity* e) {};
		virtual void onCopy(sa::Entity* e, sa::Entity* other) {};
	};

	class LuaAccessable : public ComponentBase {
	private:
		inline static std::unordered_map<std::string, std::function<sol::lua_value(sa::MetaComponent&)>> s_casters;
		inline static std::unordered_map<std::string, std::function<void(sa::MetaComponent&, sol::lua_value)>> s_copyCasters;

	public:

		static sol::state& getState();

		template<typename T, typename ...Args>
		static sol::usertype<T> registerType(const std::string& customName = "", Args&&... args);

		template<typename T>
		static void registerComponent();

		static sol::lua_value cast(sa::MetaComponent& metaComponent);
		static void copy(sa::MetaComponent& dest, sol::lua_value srcValue);

		static std::vector<std::string> getRegisteredComponents();

	};


	// ----------------------------

	template<typename T, typename ...Args>
	inline sol::usertype<T> LuaAccessable::registerType(const std::string& customName, Args&&... args) {
		return getState().new_usertype<T>((customName.empty()) ? sa::getComponentName<T>() : customName, args...);
	}

	template<typename T>
	inline void LuaAccessable::registerComponent() {
		std::string key = sa::getComponentName<T>();

		s_casters[key] = [](sa::MetaComponent& comp) -> sol::lua_value {
			return comp.cast<T>();
		};
	
		s_copyCasters[key] = [](sa::MetaComponent& mc, sol::lua_value component) {
			*mc.cast<T>() = component.as<T>();
		};
	}

	// ----------------------------


}


