#pragma once

#include "Entity.h"

namespace sa {
	class Ref {
	private:
		std::function<sol::lua_value(const sol::lua_value&)> m_indexFunction;
		std::function<void(const Ref&, Serializer&, sol::state_view)> m_serializeFunc;

		bool m_hasReference;
	public:
		Ref(const std::string& type, const Entity& value);
		Ref(const sol::table& type, const Entity& value);
		Ref(const sol::object& type, const sol::nil_t& value);



		static void reg();

	};

}
