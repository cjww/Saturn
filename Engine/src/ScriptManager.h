#pragma once

#include "Tools\utils.h"
#include "ECS/ComponentType.h"

#include "Ecs/ComponentBase.h"

#include <sol/sol.hpp>

#include "Tools/Profiler.h"

#include <Tools/Logger.hpp>
#include <filesystem>

#include "ECS/Entity.h"
#include "EntityScript.h"

namespace sa {

	class Scene;

	struct SystemScript {
		sol::environment env;
		sol::function func;
		std::vector<ComponentType> components;
	};

	

	class ScriptManager {
	public:

	private:

		std::unordered_map<std::string, SystemScript> m_systemScripts;
		
		std::unordered_map<size_t, sol::safe_function> m_scripts;
		std::unordered_map<entt::entity, std::unordered_map<std::string, size_t>> m_entityScriptIndices;
		std::vector<EntityScript> m_allScripts;
		
		template<typename ...Args>
		static void tryCall(const sol::environment& env, const sol::safe_function& function, Args&& ...args);

		void setComponents(const entt::entity& entity, sol::environment& env, std::vector<ComponentType>& components);


	public:

		ScriptManager();
		virtual ~ScriptManager();

		void loadSystemScript(const std::string& path);

		EntityScript* addScript(const Entity& entity, const std::filesystem::path& path);
		void removeScript(const entt::entity& entity, const std::string& name);
		void clearEntity(const entt::entity& entity);
		EntityScript* getScript(const entt::entity& entity, const std::string& name);

		void clearAll();
		void freeMemory();

		std::vector<EntityScript> getEntityScripts(const entt::entity& entity) const;

		template<typename ...Args>
		static void tryCall(const sol::environment& env, const std::string& functionName, Args&& ...args);
		
		template<typename ...Args>
		void broadcast(const std::string& functionName, Args&& ...args);

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
	inline void ScriptManager::broadcast(const std::string& functionName, Args&& ...args) {
		// Scripts
		for (const auto& script : m_allScripts) {
			tryCall<Args...>(script.env, functionName, args...);
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