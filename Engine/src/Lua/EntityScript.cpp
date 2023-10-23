#include "pch.h"
#include "EntityScript.h"

#include <Tools\Logger.hpp>

#include "ECS/ComponentBase.h"

namespace sa {

	void EntityScript::serialize(Serializer& s) {
		s.beginObject();
		s.value("path", path.generic_string().c_str());

		s.beginObject("env");
		for (const auto& [key, value] : serializedData) {
			SA_DEBUG_LOG_INFO("Serializing ", key);
			s.value(key, env.get<sol::object>(key));
		}
		s.endObject();

		s.endObject();
	}

	void EntityScript::deserialize(void* pDoc) {
		using namespace simdjson::ondemand;
		object& obj = *(object*)pDoc;
		auto e = obj["env"];
		for (auto o : e.get_object()) {
			auto value = o.value();
			std::string_view key = o.unescaped_key();
			SA_DEBUG_LOG_INFO("Deserializing ", key);
			switch (value.type()) {
			case json_type::number:
				if (value.get_number_type() == number_type::floating_point_number) {
					env[key] = (float)value.get_double();
				}
				else {
					env[key] = (int)value.get_int64();
				}
				break;
			case json_type::string:
				env[key] = (std::string_view)value.get_string();
				break;
			case json_type::boolean:
				env[key] = (bool)value.get_bool();
				break;
			case json_type::object:
			{
				auto field = *value.get_object().begin();
				std::string_view userTypeName= field.unescaped_key();
				simdjson::ondemand::object userDataObj = field.value().get_object();
				auto userType = LuaAccessable::getState()[userTypeName];
				if (userType == sol::nil) {
					SA_DEBUG_LOG_ERROR("Failed to deserialize object: usertype ", userTypeName, " was nil");
					break;
				}
				std::optional<sol::protected_function> deserializeFunction = userType["deserialize"].get<std::optional<sol::protected_function>>();
				if (!deserializeFunction.has_value()) {
					SA_DEBUG_LOG_ERROR("Failed to deserialize object: ", userTypeName, " has no deserialize function");
					break;
				}
				const auto func = deserializeFunction.value();
				env.set_on(func);
				const auto result = func(userDataObj);
				if (!result.valid()) {
					sol::error err = result;
					std::cout << err.what() << std::endl;
					SA_DEBUG_LOG_ERROR("Failed to deserialize object: [", userTypeName, ".deserialize] ", err.what());
					break;
				}
				env[key] = result;
				
				break;
			}
			case json_type::array:

				
			case json_type::null:
				

			default:
				throw std::runtime_error("[EntityScript deserialize] Not implemented json type");
				break;
			}

			serializedData[std::string(key)] = env[key];
			
		}
	}
}
