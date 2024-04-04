﻿#pragma once

#include "ECS/Entity.h"
#include "Lua/LuaAccessable.h"

namespace sa {
	class Ref {
	private:
		friend bool LuaAccessable::registerType<Ref>();


		std::function<sol::table()> m_retriveFunction;
		std::function<void(const Ref&, Serializer&)> m_serializeFunc;
		std::function<sol::lua_value()> m_getValueFunc;

		bool m_hasReference;
		std::string m_typeStr;
		
	public:
		Ref(const std::string& type, const Entity& value);
		Ref(const sol::table& type, const Entity& value);
		Ref(const sol::object& type, const sol::nil_t& value);
		
		bool hasReference() const;
		const std::string& getType() const;
		sol::lua_value getValue() const;

	};

}