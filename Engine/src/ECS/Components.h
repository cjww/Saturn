#pragma once
#include "ECS/Components/Name.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/Light.h"
#include "ECS/Components/Model.h"


namespace comp {
	struct Script /*: public sa::LuaAccessable */ {
		sol::environment env;
		/*
		static void reg() {
			auto type = registerType<Script>();
			type["__index"] = [](const comp::Script& script, const std::string& key) {
				return script.env[key];
			};
			type["__newindex"] = [](comp::Script& script, const std::string& key, const sol::lua_value& value) {
				script.env[key] = value;
			};

		}
		*/
	};
};

