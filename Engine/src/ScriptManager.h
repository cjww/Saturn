#pragma once

#include "Tools\utils.h"
#include "ECS/ComponentType.h"
#include "ECS/Entity.h"

#include <sol/sol.hpp>

#include <Tools/Logger.hpp>


namespace sa {

	class Scene;

	class ScriptManager {
	public:
		struct SystemScript {
			sol::environment env;
			sol::function func;
			std::vector<ComponentType> components;
		};

		struct EntityScript {
			std::string name;
			sol::environment env;
			EntityScript(std::string name, sol::environment env)
				: name(name)
				, env(env) 
			{}
		};

	private:

		std::unordered_map<std::string, SystemScript> m_systemScripts;
		
		std::unordered_map<size_t, sol::safe_function> m_scripts;
		std::unordered_map<Entity, std::unordered_map<std::string, size_t>> m_entityScriptIndices;
		std::vector<EntityScript> m_allScripts;
		
		template<typename ...Args>
		void tryCall(const sol::environment& env, const std::string& functionName, Args&& ...args);

		template<typename ...Args>
		void tryCall(const sol::environment& env, const sol::safe_function& function, Args&& ...args);

		void setComponents(const Entity& entity, sol::environment& env, std::vector<ComponentType>& components);


	public:
		ScriptManager();
		virtual ~ScriptManager();

		void loadSystemScript(const std::string& path);

		void addScript(const Entity& entity, const std::filesystem::path& path);
		void removeScript(const Entity& entity, const std::string& name);

		std::vector<EntityScript> getEntityScripts(const Entity& entity) const;

		void init(Scene* pScene);
		void update(float dt, Scene* pScene);

	};

	template<typename ...Args>
	inline void ScriptManager::tryCall(const sol::environment& env, const std::string& functionName, Args&& ...args) {
		sol::safe_function func = env[functionName];
		if (func == sol::nil) {
			return;
		}
		env.set_on(func);
		auto r = func(args...);
		if (!r.valid()) {
			SA_DEBUG_LOG_ERROR(lua_tostring(LuaAccessable::getState(), -1));
		}
	}

	template<typename ...Args>
	inline void ScriptManager::tryCall(const sol::environment& env, const sol::safe_function& function, Args&& ...args) {
		if (function == sol::nil) {
			return;
		}
		env.set_on(function);
		auto r = function(args...);
		if (!r.valid()) {
			SA_DEBUG_LOG_ERROR(lua_tostring(LuaAccessable::getState(), -1));
		}
	}
}