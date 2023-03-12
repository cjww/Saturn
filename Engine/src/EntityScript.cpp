#include "pch.h"
#include "EntityScript.h"

#include <Tools\Logger.hpp>

namespace sa {

	void EntityScript::serialize(Serializer& s) {
		s.beginObject();
		s.value("path", path.generic_string().c_str());

		s.beginObject("env");
		for (auto& [key, value] : serializedData) {
			s.value(key, (sol::object)env[key]);
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
				
				break;
			case json_type::array:
				break;
			case json_type::null:
				break;

			default:
				break;
			}
		}
			/*
			*/
	}
}