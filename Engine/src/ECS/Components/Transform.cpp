#include "pch.h"
#include "Transform.h"

#include <simdjson.h>



namespace comp {
	sa::Matrix4x4 Transform::getMatrix() const {
		return glm::translate(sa::Matrix4x4(1), position) * glm::toMat4(rotation) * glm::scale(sa::Matrix4x4(1), scale);
	}

	void Transform::serialize(sa::Serializer& s) {
		s.value("position", (glm::vec3)position);
		s.value("rotation", (glm::quat)rotation);
		s.value("scale", (glm::vec3)scale);
		s.value("hasParent", hasParent);
		s.value("relativePosition", (glm::vec3)relativePosition);
	}

	void Transform::deserialize(void* pDoc) {
		simdjson::ondemand::object& obj = *(simdjson::ondemand::object*)pDoc;
		simdjson::ondemand::object member = obj["position"].get_object();
		position = sa::Serializer::DeserializeVec3(&member);
		member = obj["rotation"].get_object();
		rotation = sa::Serializer::DeserializeQuat(&member);
		member = obj["scale"].get_object();
		scale = sa::Serializer::DeserializeVec3(&member);
		hasParent = obj["hasParent"].get_bool();
		member = obj["relativePosition"].get_object();
		relativePosition = sa::Serializer::DeserializeVec3(&member);

	}

	void Transform::reg() {
		auto type = registerType<Transform>();
		type["position"] = sol::property(
			[](Transform& self) -> sa::Vector3& {
				if (self.hasParent)
					return self.relativePosition;
				return self.position;
			},
			[](Transform& self, const sa::Vector3& value) {
				if (self.hasParent) {
					self.relativePosition = value;
					return;
				}
				self.position = value;
			}
			);
		type["rotation"] = &Transform::rotation;
		type["scale"] = &Transform::scale;
	}
}
