#include "pch.h"
#include "Light.h"

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

	void Light::reg() {
		auto type = registerType<Light>("",
			sol::constructors<Light()>());

		type["intensity"] = sol::property([](const Light& self) {return self.values.color.a; }, [](Light& self, float value) { self.values.color.a = value; });
		type["attenuationRadius"] = sol::property([](const Light& self) {return self.values.position.w; }, [](Light& self, float value) { self.values.position.w = value; });
		type["color"] = sol::property(
			[](const comp::Light& light) {return sa::Vector3(light.values.color.r, light.values.color.g, light.values.color.b); },
			[](comp::Light& self, const sa::Vector3& color) {self.values.color = sa::Color{ color.x, color.y, color.z, self.values.color.a }; }
		);

	}
}