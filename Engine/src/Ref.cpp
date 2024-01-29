#include "pch.h"
#include "Lua/Ref.h"

namespace sa {
	Ref::Ref(const std::string& type, const sa::Entity& value) {
		m_typeStr = type;
		m_getValueFunc = [=]() -> sol::lua_value { return Entity(value); };

		m_retriveFunction = [=]() -> sol::table {
			if (value.isNull())
				throw sol::error("Failed to index Ref: " + value.toString() + " is a null entity");
			const EntityScript* script = value.getScript(type);
			if (script)
				return script->env;
			throw sol::error("Failed to index Ref: " + value.toString() + " had no script named " + type);
		};

		m_serializeFunc = [=](const Ref& self, Serializer& s) {
			s.beginObject("Ref");
			s.value("type", type.c_str());
			s.value("value", (uint32_t)value);
			s.endObject();
		};

		m_hasReference = true;
	}

	Ref::Ref(const sol::table& type, const Entity& value) {
		auto getFunc = type.get<std::optional<sol::protected_function>>("Get");
		if (!getFunc.has_value())
			throw sol::error("Bad argument to Ref: Unsupported type");

		m_typeStr = LuaAccessable::getState()[type].get<std::string>();
		m_getValueFunc = [=]() -> sol::lua_value { return Entity(value); };

		m_retriveFunction = [=]() -> sol::table {
			if(value.isNull())
				throw sol::error("Failed to index Ref: " + value.toString() + " is a null entity");

			sol::userdata ud = type["Get"](value);
			return ud;
		};
		m_serializeFunc = [=](const Ref& self, Serializer& s) {
			s.beginObject("Ref");
			
			s.value("type", self.m_typeStr.c_str());
			s.value("value", (uint32_t)value);

			s.endObject();
		};
		m_hasReference = true;
	}

	Ref::Ref(const sol::object& type, const sol::nil_t& value) {
		if(type.is<sol::table>()) {
			sol::table table = type;
			auto getFunc = table.get<std::optional<sol::protected_function>>("Get");
			if (!getFunc.has_value())
				throw sol::error("Bad argument to Ref: Unsupported type");
			m_typeStr = LuaAccessable::getState()[type].get<std::string>();
		}
		else if(type.is<std::string>()) {
			m_typeStr = type.as<std::string>();
		}
		else 
			throw sol::error("Bad argument to Ref: Unsupported type");

		m_getValueFunc = [=]()  { return value; };

		m_retriveFunction = []() {
			return sol::nil;
		};

		m_serializeFunc = [=](const Ref& self, Serializer& s) {
			s.beginObject("Ref");
			s.value("type", self.m_typeStr.c_str());
			s.endObject();
		};

		m_hasReference = false;
	}

	bool Ref::hasReference() const {
		return m_hasReference;
	}

	const std::string& Ref::getType() const {
		return m_typeStr;
	}

	sol::lua_value Ref::getValue() const {
		return m_getValueFunc();
	}
}
