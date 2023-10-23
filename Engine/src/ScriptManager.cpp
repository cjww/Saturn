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

	ScriptManager::ScriptManager() {
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
		if (m_entityScriptIndices[entity].count(scriptName)) {
			return nullptr; // don't add same script again
		}

		size_t hashedString = std::hash<std::string>()(scriptName);
		sol::state& lua = LuaAccessable::getState();

		if (!m_scripts.count(hashedString)) {
			// load file as function
			auto result = lua.load_file(path.generic_string());
			if(result.status() != sol::load_status::ok) {
				sol::error err = result;
				SA_DEBUG_LOG_ERROR("Failed to load script ", path.generic_string(), ": ", err.what());
				return nullptr;
			}
			m_scripts[hashedString] = result;
		}

		sol::environment env = sol::environment(lua, sol::create, lua.globals());
		sol::safe_function& func = m_scripts[hashedString];
		env.set_on(func);

		env["entity"] = entity;
		env["scriptName"] = scriptName;
		env["scene"] = entity.getScene();

		m_allScripts.emplace_back(scriptName, path, env, entity, std::filesystem::last_write_time(path));
		m_entityScriptIndices[entity][scriptName] = m_allScripts.size() - 1;

		// fill environment with contents of script
		auto ret = func();
		if (ret.status() != sol::call_status::ok) {
			sol::error err = ret;
			SA_DEBUG_LOG_ERROR("Failed to run script ", path.generic_string(), ": ", err.what());
		}

		return &m_allScripts.back();
	}

	void ScriptManager::removeScript(const entt::entity& entity, const std::string& name) {
		size_t scriptCount = m_allScripts.size();
		auto& entityScripts = m_entityScriptIndices[entity];
		if (!entityScripts.count(name))
			return; // script does not exist on this entity
		size_t scriptIndex = entityScripts.at(name);
		if (scriptCount > 1) {
			EntityScript& backScript = m_allScripts.back();
			m_entityScriptIndices[backScript.owner][backScript.name] = scriptIndex;
			m_allScripts[scriptIndex] = std::move(backScript);
		}
		else if (scriptCount == 0)
			return;
		
		m_allScripts.pop_back();
		entityScripts.erase(name);
	}

	void ScriptManager::clearEntity(const entt::entity& entity) {
		if (!m_entityScriptIndices.count(entity))
			return;

		auto& entityScripts = m_entityScriptIndices.at(entity);
		size_t scriptCount = entityScripts.size();
		for (auto& [name, index] : entityScripts) {
			EntityScript& backScript = m_allScripts.back();
			m_entityScriptIndices[backScript.owner][backScript.name] = index;
			m_allScripts[index] = std::move(backScript);
			m_allScripts.pop_back();
		}
		m_entityScriptIndices.erase(entity);
	}

	EntityScript* ScriptManager::getScript(const entt::entity& entity, const std::string& name) const{
		if (!m_entityScriptIndices.count(entity))
			return nullptr;
		auto& entityScripts = m_entityScriptIndices.at(entity);
		if (!entityScripts.count(name))
			return nullptr;

		return (EntityScript*)&m_allScripts.at(entityScripts.at(name));
	}

	void ScriptManager::clearAll() {
		m_allScripts.clear();
		m_entityScriptIndices.clear();
		m_scripts.clear();
	}

	void ScriptManager::freeMemory() {
		m_allScripts.clear();
		m_allScripts.shrink_to_fit();

		std::unordered_map<std::string, SystemScript> tmpSystemScripts;
		m_systemScripts.swap(tmpSystemScripts);

		std::unordered_map<size_t, sol::safe_function> tmpScripts;
		m_scripts.swap(tmpScripts);

		std::unordered_map<entt::entity, std::unordered_map<std::string, size_t>> tmpEntityScriptIndices;
		m_entityScriptIndices.swap(tmpEntityScriptIndices);
		
	}

	std::vector<EntityScript*> ScriptManager::getEntityScripts(const entt::entity& entity) {
		if (!m_entityScriptIndices.count(entity))
			return {};

		std::vector<EntityScript*> scripts;
		for (auto& [name, index] : m_entityScriptIndices.at(entity)) {
			scripts.push_back(&m_allScripts[index]);
		}
		return scripts;
	}

	void ScriptManager::reloadScript(EntityScript* pScript) {
		sol::state& lua = LuaAccessable::getState();
		// do file to update environment
		auto loadResult = lua.do_file(pScript->path.generic_string(), pScript->env);

		if (loadResult.status() != sol::call_status::ok) {
			sol::error err = loadResult;
			SA_DEBUG_LOG_ERROR("Error when running script ", pScript->path.generic_string(), ": ", err.what());
		}
		SA_DEBUG_LOG_INFO("Reloaded script ", pScript->path.generic_string());
	}

	void ScriptManager::reloadScripts() {
		for(EntityScript& script : m_allScripts) {

			std::error_code error;
			std::filesystem::file_time_type time = std::filesystem::last_write_time(script.path, error);
			if(error) {
				SA_DEBUG_LOG_ERROR("Failed to load script file ", script.path);
				continue;
			}
			if(time > script.lastWriteTime) {
				reloadScript(&script);
				script.lastWriteTime = time;
			}
		}
	}
}
