#include "pch.h"
#include "Light.h"

#include <simdjson.h>

namespace comp {

	void Light::serialize(sa::Serializer& s) {
		s.value("position", (glm::vec3)values.position);
		s.value("color", *(glm::vec4*)&values.color);
		s.value("lightType", (int)values.type);
		s.value("attenuationRadius", values.attenuationRadius);
		s.value("intensity", values.intensity);
	}


	void Light::deserialize(void* pDoc) {
		simdjson::ondemand::object& obj = *(simdjson::ondemand::object*)pDoc;
		
		simdjson::ondemand::object member = obj["position"].get_object();
		values.position = sa::Serializer::DeserializeVec3(&member);
		member = obj["color"];
		glm::vec4 color = sa::Serializer::DeserializeVec4(&member);
		values.color = *(sa::Color*)&color;
		values.type = (sa::LightType)obj["lightType"].get_int64().value();
		values.attenuationRadius = obj["attenuationRadius"].get_double();
		values.intensity = obj["intensity"].get_double();
	}

	void Light::reg() {
		auto type = registerType<Light>("",
			sol::constructors<Light()>()
			);
		type["intensity"] = sol::property([](const Light& self) {return self.values.intensity; }, [](Light& self, float value) { self.values.intensity = value; });
		type["attenuationRadius"] = sol::property([](const Light& self) {return self.values.attenuationRadius; }, [](Light& self, float value) { self.values.attenuationRadius = value; });
		type["color"] = sol::property(
			[](const comp::Light& light) {return sa::Vector4(light.values.color.r, light.values.color.g, light.values.color.b, light.values.color.a); },
			[](comp::Light& self, const sa::Vector4& color) {self.values.color = sa::Color{ color.x, color.y, color.z, color.w }; }
		);

	}
}