#pragma once

#include "Tools\utils.h"
#include "ECS/ComponentType.h"
#include "ECS/Entity.h"

#include <sol/sol.hpp>
#include "Scene.h"

#include <Tools/Logger.hpp>


namespace sa {

	class ScriptManager {
	public:
		struct Script {
			sol::environment env;
			sol::function func;
			std::vector<ComponentType> components;
		};

	private:

		std::unordered_map<std::string, Script> m_scripts;
		std::unordered_map<size_t, sol::table> m_customComponents;


		sol::state m_lua;

		std::unordered_map<std::string, std::function<sol::lua_value(MetaComponent&)>> m_componentCasters;

		sol::lua_value castComponent(MetaComponent& metaComp);

		template<typename ...Args>
		void tryCall(const sol::environment& env, const std::string& functionName, Args&& ...args);

		void setComponents(const Entity& entity, sol::environment& env, std::vector<ComponentType>& components);

		void registerComponentType(std::string name, sol::table componentData);


	public:
		ScriptManager();
		virtual ~ScriptManager();

		void load(const std::string& path);

		void init(Scene* pScene);
		void update(float dt, Scene* pScene);

		template<typename T>
		sol::usertype<T> registerComponent();

		template<typename T>
		sol::usertype<T> registerType();

		sol::usertype<Entity> registerEntityType();


	};

	template<typename ...Args>
	inline void ScriptManager::tryCall(const sol::environment& env, const std::string& functionName, Args&& ...args) {
		sol::safe_function func = env[functionName];
		if (func != sol::nil) {
			env.set_on(func);
			auto r = func(args...);
			if (!r.valid()) {
				DEBUG_LOG_ERROR(lua_tostring(m_lua, -1));
			}
		}
	}
	
	template<typename T>
	inline sol::usertype<T> ScriptManager::registerComponent() {
		m_componentCasters[getComponentName<T>()] = [](MetaComponent& comp) -> sol::lua_value {
			return comp.cast<T>();
		};
		return registerType<T>();
	}

	template<typename T>
	inline sol::usertype<T> ScriptManager::registerType() {
		return m_lua.new_usertype<T>(getComponentName<T>());
	}
}