#include "pch.h"
#include "ScriptManager.h"
#include "ECS/Components.h"
namespace sa {

	ScriptManager::ScriptManager() {
		m_lua.open_libraries();
		auto ret = m_lua.do_string("return 'Loaded ' .. jit.version .. ' for ' .. jit.os .. ' ' .. jit.arch");
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
		m_scripts.emplace_back();
		Script& script = m_scripts.back();
		script.env = std::move(sol::environment(m_lua, sol::create, m_lua.globals()));

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

		script.func = m_lua.load_file(path);

		m_lua.stack_clear();
	}
	
	void ScriptManager::update() {
		
	}

	std::vector<ScriptManager::Script>& ScriptManager::getScripts()
	{
		return m_scripts;
	}

	sol::state& ScriptManager::getState() {
		return m_lua;
	}
}
