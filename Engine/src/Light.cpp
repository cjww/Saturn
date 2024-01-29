#include "pch.h"
#include "ECS/Components/Light.h"
namespace sa {
	const char* to_string(LightType type) {
		switch (type) {
		case LightType::POINT:
			return "Point light";
		case LightType::DIRECTIONAL:
			return "Directional light";
		case LightType::SPOT:
			return "Spotlight";
		default:
			return "None";
		}
	}
}

namespace comp {


	void Light::serialize(sa::Serializer& s) {
		s.value("color", *(glm::vec4*)&values.color);
		s.value("position", values.position);
		s.value("direction", values.direction);
		s.value("lightType", (int)values.type);
	}


	void Light::deserialize(void* pDoc) {
		simdjson::ondemand::object& obj = *(simdjson::ondemand::object*)pDoc;
		
		simdjson::ondemand::object member = obj["color"].get_object();
		glm::vec4 color = sa::Serializer::DeserializeVec4(&member);
		values.color = *(sa::Color*)&color;
		
		member = obj["position"];
		values.position = sa::Serializer::DeserializeVec4(&member);

		auto field = obj.find_field("direction");
		if (field.error() != simdjson::NO_SUCH_FIELD) {
			member = field;
			values.direction = sa::Serializer::DeserializeVec4(&member);
		}
		

		values.type = (sa::LightType)obj["lightType"].get_int64().value();
	}
}
