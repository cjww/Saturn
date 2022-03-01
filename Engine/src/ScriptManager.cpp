#include "pch.h"
#include "ScriptManager.h"
#include "ECS/Components.h"

namespace sa {
	sol::lua_value ScriptManager::castComponent(MetaComponent& metaComp) {
		return m_componentCasters[metaComp.getTypeName()](metaComp);
	}

	
	void ScriptManager::setComponents(const Entity& entity, sol::environment& env, std::vector<ComponentType>& components) {
		for (auto& type : components) {
			auto metaComp = type.invoke("get", entity);
			std::string name = utils::toLower(type.getName());
			env[name] = LuaAccessable::cast(metaComp);
		}
	}

	ScriptManager::ScriptManager() {
		LuaAccessable::getState().open_libraries();
		//m_lua.open_libraries();

		auto ret = LuaAccessable::getState().do_string("return 'Loaded ' .. jit.version .. ' for ' .. jit.os .. ' ' .. jit.arch");
		//auto ret = m_lua.do_string("return 'Loaded ' .. jit.version .. ' for ' .. jit.os .. ' ' .. jit.arch");
		if (ret.valid()) {
			DEBUG_LOG_INFO(ret[0].as<std::string>());	
		}
		else {
			DEBUG_LOG_WARNING("Failed to load luaJit");
		}


	}

	ScriptManager::~ScriptManager() {
		m_scripts.clear();
	}


	void ScriptManager::load(const std::string& path) {
		sol::state& lua = LuaAccessable::getState();


		size_t size = m_scripts.size();
		Script& script = m_scripts[path];

		if (size != m_scripts.size()) {
			script.env = std::move(sol::environment(lua, sol::create, lua.globals()));
		}
		script.components.clear();
		ComponentType scriptType = getComponentType<comp::Script>();
		if (!scriptType.isValid()) {
			registerComponentType<comp::Script>();
			scriptType = getComponentType<comp::Script>();
		}
		script.components.push_back(scriptType);

		std::ifstream file(path);
		if (!file.is_open() || file.eof()) {
			file.close();
			return;
		}

		// Get first line with content i lua file
		std::string line;
		do {
			std::getline(file, line);
		} while (!file.eof() && line.size() == 0);
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
						DEBUG_LOG_WARNING(word, "was not a recognized component type, ignoring...");
					}
				}
			}
		}

		script.func = lua.load_file(path);

		script.env.set_on(script.func);

		auto ret = script.func();
		if (!ret.valid()) {
			DEBUG_LOG_ERROR(lua_tostring(lua, -1));
		}

		lua.stack_clear();
	}
	
	void ScriptManager::init(Scene* pScene) {
		sol::state& lua = LuaAccessable::getState();
		for (auto& pair : m_scripts) {
			Script& script = pair.second;

			pScene->forEach(script.components, [&](const Entity& entity)
				{
					comp::Script* scriptComp = entity.getComponent<comp::Script>();
					if(!scriptComp->env.valid()) {
						scriptComp->env = sol::environment(lua, sol::create, script.env);
					}
					setComponents(entity, scriptComp->env, script.components);
					scriptComp->env["entity"] = entity;
					tryCall(scriptComp->env, "init");
				});
			
		}
	}

	void ScriptManager::update(float dt, Scene* pScene) {
		for (auto& pair : m_scripts) {
			Script& script = pair.second;

			pScene->forEach(script.components, [&](const Entity& entity)
				{
					comp::Script* scriptComp = entity.getComponent<comp::Script>();
					tryCall(scriptComp->env, "update", dt);
				});
			
		}
	}


	
}
