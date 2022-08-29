#pragma once

#include "sol/sol.hpp"
#include "ComponentType.h"

namespace sa {
	class Entity;

	class ComponentBase {
	public:
		

	};

	class LuaAccessable : public ComponentBase {
	private:
		inline static std::unordered_map<std::string, std::function<sol::lua_value(sa::MetaComponent&)>> s_casters;
	public:

		static sol::state& getState();

		template<typename T, typename ...Args>
		static sol::usertype<T> luaReg(const std::string& customName = "", Args&&... args);

		template<typename T>
		static sol::usertype<T> registerComponent();

		static sol::lua_value cast(sa::MetaComponent& metaComponent);

		static std::vector<std::string> getRegisteredComponents();

	};


	// ----------------------------

	template<typename T, typename ...Args>
	inline sol::usertype<T> LuaAccessable::luaReg(const std::string& customName, Args&&... args) {
		return getState().new_usertype<T>((customName.empty()) ? sa::getComponentName<T>() : customName, args...);
	}

	template<typename T>
	inline sol::usertype<T> LuaAccessable::registerComponent() {
		s_casters[sa::getComponentName<T>()] = [](sa::MetaComponent& comp) -> sol::lua_value {
			return comp.cast<T>();
		};
		return luaReg<T>();
	}

	

	// ----------------------------


}


