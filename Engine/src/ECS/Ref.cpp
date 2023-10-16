#include "pch.h"
#include "Ref.h"

#include "Components/Name.h"

namespace sa {
	Ref::Ref(const std::string& type, const sa::Entity& value) {
		m_indexFunction = [=](const sol::lua_value& key) -> sol::lua_value {
			if(value.isNull())
				throw sol::error("Failed to index Ref: " + value.toString() + " is a null entity");
			Entity entity = value;
			std::string scriptName = type;
			EntityScript* script = entity.getScript(scriptName);
			if (script)
				return script->env[key];
			throw sol::error("Failed to index Ref: " + value.toString() + " had no script named " + scriptName);
		};

		m_serializeFunc = [=](const Ref& self, Serializer& s, sol::state_view) {
			s.beginObject("Ref");
			s.value("type", type.c_str());
			s.value("value", (uint32_t)value);
			s.endObject();
		};

		m_hasReference = true;
	}

	Ref::Ref(const sol::table& type, const Entity& value) {
		auto getFunc = type.get<std::optional<sol::protected_function>>("get");
		if (!getFunc.has_value())
			throw sol::error("Bad argument to Ref: Unsupported type");

		m_indexFunction = [=](const sol::lua_value& key) -> sol::lua_value {
			if(value.isNull())
				throw sol::error("Failed to index Ref: " + value.toString() + " is a null entity");

			sol::userdata ud = type["get"](value);
			return ud[key];
		};
		m_serializeFunc = [=](const Ref& self, Serializer& s, sol::state_view lua) {
			s.beginObject("Ref");
			
			s.value("type", lua[type].get<std::string>().c_str());
			s.value("value", (uint32_t)value);

			s.endObject();
		};
		m_hasReference = true;
	}

	Ref::Ref(const sol::object& type, const sol::nil_t& value) {
		if(type.is<sol::table>()) {
			sol::table table = type;
			auto getFunc = table.get<std::optional<sol::protected_function>>("get");
			if (!getFunc.has_value())
				throw sol::error("Bad argument to Ref: Unsupported type");
		}
		else if(!type.is<std::string>()) {
			throw sol::error("Bad argument to Ref: Unsupported type");
		}

		m_indexFunction = [](const sol::lua_value&) { return sol::nil; };
		m_serializeFunc = [=](const Ref& self, Serializer& s, sol::state_view lua) {
			s.beginObject("Ref");

			switch (type.get_type())
			{
				
			case sol::type::table:
				s.value("type", lua[type].get<std::string>().c_str());
				break;
			
			case sol::type::string:
				s.value("type", type.as<std::string>().c_str());
				break;
			default:
				{
					std::string str = lua["tostring"](type);
					s.value("type", str.c_str());
					break;
				}
			}

			s.endObject();
		};
		m_hasReference = false;
	}

	void Ref::reg() {
		auto type = LuaAccessable::registerType<Ref>("Ref",
			sol::constructors<Ref(const std::string&, const Entity&), Ref(const sol::table&, const Entity&), Ref(const sol::object&, const sol::nil_t&)>()
			);

		type["hasReference"] = [](const Ref& self) {
			return self.m_hasReference;
		};

		type["__index"] = [](Ref& self, const sol::lua_value& key) -> sol::lua_value {
			return self.m_indexFunction(key);
		};
		
		type["serialize"] = [](const Ref& self, Serializer& s, sol::this_state ts) {
			self.m_serializeFunc(self, s, ts);
		};

		type["deserialize"] = [](simdjson::ondemand::object& jsonObject, sol::this_state ts, sol::this_environment te) -> sol::lua_value {
			const sol::state_view& lua = ts;
			sol::environment& env = te;

			sol::lua_value type(lua, sol::nil);
			sol::lua_value value(lua, sol::nil);

			auto typeResult = jsonObject.find_field("type");
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

			return lua["Ref"]["new"](type, value);
		};
	}
}
