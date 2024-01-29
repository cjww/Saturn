#pragma once
#include "ECS/ComponentBase.h"
#include "ECS/MetaComponent.h"
#include "ECS/ComponentType.h"
#include "sol/sol.hpp"

namespace sa {

	class LuaAccessable {
	private:
		inline static std::unordered_map<std::string, std::function<sol::lua_value(sa::MetaComponent&)>> s_casters;
		inline static std::unordered_map<std::string, std::function<void(sa::MetaComponent&, sol::lua_value)>> s_copyCasters;

	public:

		static sol::state& getState();

		template<typename T, typename ...Args>
		static sol::usertype<T>& userType(const std::string& customName = "", Args&&... args);



		template<typename Comp>
		static void registerComponent();

		static sol::lua_value cast(sa::MetaComponent& metaComponent);
		static void copy(sa::MetaComponent& dest, sol::lua_value srcValue);

		static std::vector<std::string> getRegisteredComponents();



		template<typename T>
		static bool registerType();
	};


	// ----------------------------

	template<typename T, typename ...Args>
	inline sol::usertype<T>& LuaAccessable::userType(const std::string& customName, Args&&... args) {
		std::string name = (customName.empty()) ? sa::getComponentName<T>() : customName;
		static auto type = getState().new_usertype<T>(name, args...);
		getState().set(type, name);
		return type;
	}

	template<typename Comp>
	inline void LuaAccessable::registerComponent() {
		std::string key = sa::getComponentName<Comp>();

		s_casters[key] = [](sa::MetaComponent& comp) -> sol::lua_value {
			return comp.cast<Comp>();
		};

		s_copyCasters[key] = [](sa::MetaComponent& mc, sol::lua_value component) {
			*mc.cast<Comp>() = component.as<Comp>();
		};
	}


	template <typename T>
	bool LuaAccessable::registerType() {
		return false;
	}

	// ----------------------------

}
