#include "pch.h"
#include "ScriptManager.h"
#include "ECS/Components.h"
#include "Scene.h"

namespace sa {
	
	void ScriptManager::setComponents(const Entity& entity, sol::environment& env, std::vector<ComponentType>& components) {
		for (auto& type : components) {
			auto metaComp = type.invoke("get", entity);
			std::string name = utils::toLower(type.getName());
			env[name] = LuaAccessable::cast(metaComp);
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

	void ScriptManager::addScript(const Entity& entity, const std::filesystem::path& path) {
		if (!std::filesystem::exists(path)) {
			SA_DEBUG_LOG_ERROR("File does not exist:", path);
			return;
		}


		std::string scriptName = path.filename().replace_extension().string();
		if (m_entityScriptIndices[entity].count(scriptName)) {
			return; // don't add same script again
		}

		size_t hashedString = std::hash<std::string>()(scriptName);
		sol::state& lua = LuaAccessable::getState();

		if (!m_scripts.count(hashedString)) {
			// load file as function
			m_scripts[hashedString] = lua.load_file(path.string());
		}

		sol::environment env = sol::environment(lua, sol::create, lua.globals());
		sol::safe_function& func = m_scripts[hashedString];
		env.set_on(func);

		// fill environment with contents of script
		auto ret = func();
		if (!ret.valid()) {
			SA_DEBUG_LOG_ERROR(lua_tostring(lua, -1));
		}

		env["entity"] = entity;
		m_allScripts.emplace_back(scriptName, env);
		m_entityScriptIndices[entity][scriptName] = m_allScripts.size() - 1;

	}

	void ScriptManager::removeScript(const Entity& entity, const std::string& name) {
		auto& entityScripts = m_entityScriptIndices[entity];
		if (!entityScripts.count(name))
			return; // script does not exist on this entity
		size_t scriptIndex = entityScripts[name];
		m_allScripts.erase(m_allScripts.begin() + scriptIndex);
		entityScripts.erase(name);
		for (auto& [name, index] : entityScripts) {
			if (index > scriptIndex)
				index--;
		}
	}

	std::vector<ScriptManager::EntityScript> ScriptManager::getEntityScripts(const Entity& entity) const {
		if (!m_entityScriptIndices.count(entity))
			return {};

		std::vector<EntityScript> scripts;
		for (auto& [name, index] : m_entityScriptIndices.at(entity)) {
			scripts.push_back(m_allScripts[index]);
		}
		return std::move(scripts);
	}
	
	void ScriptManager::init(Scene* pScene) {
		SA_PROFILE_FUNCTION();
		sol::state& lua = LuaAccessable::getState();
		// System scripts
		for (auto& pair : m_systemScripts) {
			SystemScript& script = pair.second;

			pScene->forEach(script.components, [&](const Entity& entity)
				{
					comp::Script* scriptComp = entity.getComponent<comp::Script>();
					if(!scriptComp->env.valid()) {
						scriptComp->env = sol::environment(lua, sol::create, lua.globals());
					}
					setComponents(entity, scriptComp->env, script.components);
					scriptComp->env["entity"] = entity;

					sol::safe_function f = script.env["init"];
					tryCall(scriptComp->env, f);
				});
			
		}

		// Scripts
		for (auto& script : m_allScripts) {
			tryCall(script.env, "init");
		}
	}

	void ScriptManager::update(float dt, Scene* pScene) {
		SA_PROFILE_FUNCTION();
		// System scripts
		for (auto& pair : m_systemScripts) {
			SystemScript& script = pair.second;

			pScene->forEach(script.components, [&](const Entity& entity)
				{
					comp::Script* scriptComp = entity.getComponent<comp::Script>();
					if (!scriptComp->env.valid()) {
						sol::state& lua = LuaAccessable::getState();
						scriptComp->env = sol::environment(lua, sol::create, lua.globals());
						
						setComponents(entity, scriptComp->env, script.components);
						scriptComp->env["entity"] = entity;
						sol::safe_function f = script.env["init"];
						tryCall(scriptComp->env, f);
					}

					tryCall(scriptComp->env, "update", dt);
				});
			
		}

		// Scripts
		for (auto& script : m_allScripts) {
			tryCall(script.env, "update", dt);
		}
	}


	
}
