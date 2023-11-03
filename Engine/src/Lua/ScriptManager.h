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

	struct SystemScript {
		sol::environment env;
		sol::function func;
		std::vector<ComponentType> components;
	};

	class ScriptManager {
	private:

		std::unordered_map<std::string, SystemScript> m_systemScripts;
		std::unordered_map<entt::entity, std::unordered_map<std::string, EntityScript>> m_entityScripts;
		
		std::list<EntityScript*> m_scriptsToBind;

		entt::emitter<Scene>& m_eventEmitter;
		
		void setComponents(const entt::entity& entity, sol::environment& env, std::vector<ComponentType>& components);
			
		void connectCallbacks(EntityScript* pScript);

		/*
		template<typename Event, typename ...Args>
		std::optional<entt::emitter<Scene>::connection<Event>> callback(EntityScript* pScript, const std::string& functionName, Args&&...);
		template <typename Event>
		std::optional<entt::emitter<Scene>::connection<Event>> callback(EntityScript* pScript, const std::string& functionName, const std::function<void(const Event&, Scene&)>& wrapper)
		*/

	public:

		ScriptManager(entt::emitter<Scene>& eventEmitter);
		virtual ~ScriptManager();

		void loadSystemScript(const std::string& path);

		EntityScript* addScript(const Entity& entity, const std::filesystem::path& path);
		void removeScript(const entt::entity& entity, const std::string& name);
		void clearEntity(const entt::entity& entity);
		EntityScript* getScript(const entt::entity& entity, const std::string& name) const;

		// Binds created scripts to callbacks and removes script queued to be removed. Can not be called in a event listener function
		void applyChanges();

		void clearAll();
		void freeMemory();

		std::vector<EntityScript*> getEntityScripts(const entt::entity& entity);
		void reloadScript(EntityScript* pScript);
		void reloadScripts();


		template<typename ...Args>
		static void tryCall(const sol::environment& env, const std::string& functionName, Args&& ...args);

		template<typename ...Args>
		static void call(const sol::safe_function func, Args&& ...args);
		

	};
	/*
	template <typename Event, typename ... Args>
	inline std::optional<entt::emitter<Scene>::connection<Event>> ScriptManager::callback(EntityScript* pScript,
		const std::string& functionName, Args&&... args)
	{
		std::optional<entt::emitter<Scene>::connection<Event>> conn;
		sol::safe_function function = pScript->env[functionName];
		pScript->env.set_on(function);
		if (function != sol::nil) {
			conn = m_eventEmitter.on<Event>([=](const Event& e, Scene&) {
				call(function, (e->*args)... );
			});
		}
		return conn;
	}

	template <typename Event>
	inline std::optional<entt::emitter<Scene>::connection<Event>> ScriptManager::callback(EntityScript* pScript,
		const std::string& functionName, const std::function<void(const Event&, Scene&)>& wrapper)
	{
		std::optional<entt::emitter<Scene>::connection<Event>> conn;
		sol::safe_function function = pScript->env[functionName];
		pScript->env.set_on(function);
		if (function != sol::nil) {
			conn = m_eventEmitter.on<Event>(wrapper);
		}
		return conn;
	}
	*/

	template<typename ...Args>
	inline void ScriptManager::tryCall(const sol::environment& env, const std::string& functionName, Args&& ...args) {
		sol::safe_function function = env[functionName];
		if (function == sol::nil) {
			return;
		}
		env.set_on(function);
		auto result = function(args...);
		if (result.status() != sol::call_status::ok) {
			sol::error error = result;
			SA_DEBUG_LOG_ERROR("Error in lua function: ", error.what());
		}
	}

	template <typename ... Args>
	void ScriptManager::call(const sol::safe_function func, Args&&... args) {
		auto result = func(args...);
		if (result.status() != sol::call_status::ok) {
			sol::error error = result;
			SA_DEBUG_LOG_ERROR("Error in lua function: ", error.what());
		}
	}

}