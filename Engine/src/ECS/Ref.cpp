#include "pch.h"
#include "Ref.h"

#include "Components/Name.h"

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

	void Ref::reg() {
		auto type = LuaAccessable::registerType<Ref>("Ref",
			sol::call_constructor, 
			sol::constructors<Ref(const std::string&, const Entity&), Ref(const sol::table&, const Entity&), Ref(const sol::object&, const sol::nil_t&)>());

		type["hasReference"] = [](const Ref& self) {
			return self.m_hasReference;
		};

		type["__index"] = [](Ref& self, const sol::lua_value& key) -> sol::lua_value {
			sol::lua_value v = self.m_retriveFunction()[key];
			if (v.is<sol::function>())
				throw sol::error("Referencing functions from Ref is not allowed. Use ref.value." + key.as<std::string>() + " instead");
			return v;
		};

		type["value"] = sol::property([](Ref& self) -> sol::lua_value {
			return self.m_retriveFunction();
		});

		
		type["serialize"] = [](const Ref& self, Serializer& s) {
			self.m_serializeFunc(self, s);
		};
		
		type["deserialize"] = [](simdjson::ondemand::object& jsonObject, sol::this_state ts, sol::this_environment te) -> sol::lua_value {
			const sol::state_view& lua = ts;
			sol::environment& env = te;

			sol::lua_value type(lua, sol::nil);
			sol::lua_value value(lua, sol::nil);

			auto typeResult = jsonObject.find_field("type");
			if (typeResult.error() != simdjson::error_code::SUCCESS)
				throw std::runtime_error("[Ref.deserialize]: Could not find json field \"type\"");

			std::string_view typeStr = typeResult.get_string().take_value();
			
			if (lua[typeStr] != sol::nil)
				type = lua[typeStr];
			else
				type = typeStr;

			auto valueResult = jsonObject.find_field("value");
			if(valueResult.error() == simdjson::error_code::SUCCESS) {
				if (valueResult.is_integer()) {
					int64_t id = valueResult.get_int64().take_value();
					Scene* pScene = env["scene"];
					Entity entity(pScene, static_cast<entt::entity>(id));
					value = entity;
				}
				else
					value = static_cast<uint64_t>(valueResult.get_int64_in_string().take_value());
			}

			return lua["Ref"](type, value);
		};
	}
}
