#include "pch.h"
#include "LuaAccessable.h"

namespace sa {
	sol::state& LuaAccessable::getState() {
		static sol::state luaState;
		return luaState;
	}

	sol::lua_value LuaAccessable::cast(sa::MetaComponent& metaComponent) {
		return s_casters[metaComponent.getTypeName()](metaComponent);
	}

	void LuaAccessable::copy(sa::MetaComponent& dest, sol::lua_value srcValue) {
		s_copyCasters[dest.getTypeName()](dest, srcValue);
	}

	std::vector<std::string> LuaAccessable::getRegisteredComponents() {
		std::vector<std::string> names;
		names.reserve(s_casters.size());
		for (auto& [key, value] : s_casters) {
			names.push_back(key);
		}
		return names;
	}
}
