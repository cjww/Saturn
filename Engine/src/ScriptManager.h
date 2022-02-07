#pragma once

#include "Tools\utils.h"
#include "ECS/ComponentType.h"
#include "ECS/Entity.h"

#include <sol/sol.hpp>

namespace sa {

	class ScriptManager {
	public:
		struct Script {
			sol::environment env;
			sol::function func;
			std::vector<ComponentType> components;

		};
	private:

		std::vector<Script> m_scripts;
		sol::state m_lua;
	
	public:
		ScriptManager();
		virtual ~ScriptManager();

		void load(const std::string& path);

		void update();

		std::vector<Script>& getScripts();
		sol::state& getState();


		template<typename T>
		sol::usertype<T> registerComponent();

		template<typename T>
		sol::usertype<T> registerType();

	};
	
	template<typename T>
	inline sol::usertype<T> ScriptManager::registerComponent() {
		return registerType<T>();
	}

	template<typename T>
	inline sol::usertype<T> ScriptManager::registerType() {
		return m_lua.new_usertype<T>(getComponentName<T>());
	}
}