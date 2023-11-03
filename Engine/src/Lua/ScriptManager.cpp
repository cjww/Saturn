#include "pch.h"
#include "ScriptManager.h"
#include "ECS/Components.h"
#include "Scene.h"

namespace sa {
	
	void ScriptManager::setComponents(const entt::entity& entity, sol::environment& env, std::vector<ComponentType>& components) {
		for (auto& type : components) {
			/*
			auto metaComp = type.invoke("get", entity);
			std::string name = utils::toLower(type.getName());
			env[name] = LuaAccessable::cast(metaComp);
			*/
		}
	}

	void ScriptManager::connectCallbacks(EntityScript* pScript) {
		std::optional<entt::emitter<Scene>::connection<scene_event::SceneUpdate>> updateConn;
		{
			sol::safe_function function = pScript->env["onUpdate"];
			pScript->env.set_on(function);
			if (function != sol::nil) {
				updateConn = m_eventEmitter.on<scene_event::SceneUpdate>([=](const scene_event::SceneUpdate& e, Scene&) {
					call(function, e.deltaTime);
				});
			}
		}
		std::optional<entt::emitter<Scene>::connection<scene_event::SceneStart>> startConn;
		{
			sol::safe_function function = pScript->env["onStart"];
			pScript->env.set_on(function);
			if (function != sol::nil) {
				startConn = m_eventEmitter.on<scene_event::SceneStart>([=](const scene_event::SceneStart&, Scene&) {
					call(function);
				});
			}
		}

		std::optional<entt::emitter<Scene>::connection<scene_event::SceneStop>> stopConn;
		{
			sol::safe_function function = pScript->env["onStop"];
			pScript->env.set_on(function);
			if (function != sol::nil) {
				stopConn = m_eventEmitter.on<scene_event::SceneStop>([=](const scene_event::SceneStop&, Scene&) {
					call(function);
				});
			}
		}
		
		
		pScript->disconnectCallbacks = [=]() {
			if (updateConn.has_value()) m_eventEmitter.erase(updateConn.value());
			if (startConn.has_value()) m_eventEmitter.erase(startConn.value());
			if (stopConn.has_value()) m_eventEmitter.erase(stopConn.value());

		};

	}

	ScriptManager::ScriptManager(entt::emitter<Scene>& eventEmitter)
		: m_eventEmitter(eventEmitter)
	{
		SA_PROFILE_FUNCTION();
		
		LuaAccessable::getState().open_libraries();

		auto ret = LuaAccessable::getState().do_string("return 'Loaded ' .. jit.version .. ' for ' .. jit.os .. ' ' .. jit.arch");
		if (ret.valid()) {
			SA_DEBUG_LOG_INFO(ret[0].as<std::string>());	
		}
		else {
			SA_DEBUG_LOG_WARNING("Failed to load luaJit");
		}
	}

	ScriptManager::~ScriptManager() {
		m_systemScripts.clear();
	}


	void ScriptManager::loadSystemScript(const std::string& path) {
		SA_PROFILE_FUNCTION();

		sol::state& lua = LuaAccessable::getState();


		size_t size = m_systemScripts.size();
		SystemScript& script = m_systemScripts[path];

		if (size != m_systemScripts.size()) {
			script.env = sol::environment(lua, sol::create, lua.globals());
		}
		script.components.clear();
		ComponentType scriptType = getComponentType<comp::Script>();
		if (!scriptType.isValid()) {
			registerComponentType<comp::Script>();
			scriptType = getComponentType<comp::Script>();
		}
		script.components.push_back(scriptType);

		std::ifstream file(path);
		if (!file.is_open()) {
			file.close();
			SA_DEBUG_LOG_ERROR("Could not open lua file: ", path);
			return;
		}
		
		// Get first line with content i lua file
		std::string line;
		while (!file.eof() && line.size() == 0) {
			std::getline(file, line);
		}
		file.close();


		std::vector<std::string> words = utils::split(line, ' '); // split the line to words

		if (words[0].find("--") != std::string::npos) { // This is a lua comment
			if (line.find("$using") == std::string::npos) { // look for keyword
				return;
			}

			for (auto& word : words) {
				if (word[0] != '-' && word[0] != '$') {
					word = utils::toLower(word);
					word[0] = utils::toUpper(word[0]);
					
					ComponentType type = getComponentType(word);
					if (type.isValid()) {
						script.components.push_back(type);
					}
					else {
						SA_DEBUG_LOG_WARNING(word, "was not a recognized component type, ignoring...");
					}
				}
			}
		}

		script.func = lua.load_file(path);

		script.env.set_on(script.func);

		auto ret = script.func();
		if (!ret.valid()) {
			SA_DEBUG_LOG_ERROR(lua_tostring(lua, -1));
		}

		lua.stack_clear();
	}

	EntityScript* ScriptManager::addScript(const Entity& entity, const std::filesystem::path& path) {
		if (!std::filesystem::exists(path)) {
			SA_DEBUG_LOG_ERROR("File does not exist: ", path);
			return nullptr;
		}

		std::string scriptName = path.filename().replace_extension().generic_string();
		
		if (m_entityScripts[entity].count(scriptName)) {
			return &m_entityScripts[entity][scriptName]; // don't add same script again
		}

		sol::state& lua = LuaAccessable::getState();

		// load file as function
		auto result = lua.load_file(path.generic_string());
		if(result.status() != sol::load_status::ok) {
			sol::error err = result;
			SA_DEBUG_LOG_ERROR("Failed to load script ", path.generic_string(), ": ", err.what());
			return nullptr;
		}

		sol::environment env = sol::environment(lua, sol::create, lua.globals());
		const sol::safe_function func = result;
		env.set_on(func);

		env["this_entity"] = entity;
		env["this_name"] = scriptName;
		env["this_scene"] = entity.getScene();

		m_entityScripts[entity].insert({ scriptName,
			EntityScript(scriptName, path, env, entity, std::filesystem::last_write_time(path)) });

		m_scriptsToBind.emplace_back(&m_entityScripts[entity][scriptName]);
		
		// fill environment with contents of script
		auto ret = func();
		if (ret.status() != sol::call_status::ok) {
			sol::error err = ret;
			SA_DEBUG_LOG_ERROR("Failed to run script ", path.generic_string(), ": ", err.what());
		}
		
		 
		return &m_entityScripts[entity][scriptName];
	}

	void ScriptManager::removeScript(const entt::entity& entity, const std::string& name) {
		
		auto& entityScripts = m_entityScripts[entity];
		if (!entityScripts.count(name))
			return; // script does not exist on this entity

		EntityScript& script = entityScripts.at(name);
		script.disconnectCallbacks();

		entityScripts.erase(name);
		
	}

	void ScriptManager::clearEntity(const entt::entity& entity) {
		if (!m_entityScripts.count(entity))
			return;

		auto& entityScripts = m_entityScripts.at(entity);
		for (auto& [name, script] : entityScripts) {
			script.disconnectCallbacks();
		}
		m_entityScripts.erase(entity);
	}

	EntityScript* ScriptManager::getScript(const entt::entity& entity, const std::string& name) const{
		if (!m_entityScripts.count(entity))
			return nullptr;
		auto& entityScripts = m_entityScripts.at(entity);
		if (!entityScripts.count(name))
			return nullptr;

		return (EntityScript*)&entityScripts.at(name);
	}

	void ScriptManager::applyChanges() {
		for(auto& pScript: m_scriptsToBind) {
			connectCallbacks(pScript);
		}
		
		m_scriptsToBind.clear();

	}

	void ScriptManager::clearAll() {
		
		for (auto [entity, scripts] : m_entityScripts) {
			for (auto& [name, script] : scripts) {
				script.disconnectCallbacks();
			}
		}
		m_entityScripts.clear();

	}

	void ScriptManager::freeMemory() {
		
		std::unordered_map<std::string, SystemScript> tmpSystemScripts;
		m_systemScripts.swap(tmpSystemScripts);

		std::unordered_map<entt::entity, std::unordered_map<std::string, EntityScript>> tmpEntityScripts;
		m_entityScripts.swap(tmpEntityScripts);
		
	}

	std::vector<EntityScript*> ScriptManager::getEntityScripts(const entt::entity& entity) {
		if (!m_entityScripts.count(entity))
			return {};

		std::vector<EntityScript*> scripts;
		for (auto& [name, script] : m_entityScripts.at(entity)) {
			scripts.push_back(&script);
		}
		return scripts;
	}

	void ScriptManager::reloadScript(EntityScript* pScript) {
		sol::state& lua = LuaAccessable::getState();
		// load file to store function
		auto loadResult = lua.load_file(pScript->path.generic_string());
		if (loadResult.status() != sol::load_status::ok) {
			sol::error err = loadResult;
			SA_DEBUG_LOG_ERROR("Error when loading script ", pScript->path.generic_string(), ": ", err.what());
			return;
		}

		// do file to update environment
		sol::safe_function func = loadResult;
		pScript->env.set_on(func);
		auto callResult = func();

		if (callResult.status() != sol::call_status::ok) {
			sol::error err = callResult;
			SA_DEBUG_LOG_ERROR("Error when running script ", pScript->path.generic_string(), ": ", err.what());
			return;
		}

		pScript->disconnectCallbacks();
		connectCallbacks(pScript);

		SA_DEBUG_LOG_INFO("Reloaded script ", pScript->path.generic_string());
	}

	void ScriptManager::reloadScripts() {
		for(auto& [entity, scripts] : m_entityScripts) {
			for (auto& [name, script] : scripts) {
				std::error_code error;
				std::filesystem::file_time_type time = std::filesystem::last_write_time(script.path, error);
				if (error) {
					SA_DEBUG_LOG_ERROR("Failed to load script file ", script.path);
					continue;
				}
				if (time > script.lastWriteTime) {
					reloadScript(&script);
					script.lastWriteTime = time;
				}
			}
		}
	}
}
