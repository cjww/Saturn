#include "pch.h"
#include "ECS/Components/Transform.h"

namespace comp {
	Transform::Transform(physx::PxTransform pxTransform) {
		scale = sa::Vector3(1);
		position = { pxTransform.p.x, pxTransform.p.y, pxTransform.p.z };
		rotation = { pxTransform.q.x, pxTransform.q.y, pxTransform.q.z, pxTransform.q.w };
	}

	Transform& Transform::operator=(const physx::PxTransform pxTransform) {
		position = { pxTransform.p.x, pxTransform.p.y, pxTransform.p.z };
		rotation = glm::quat(pxTransform.q.w, pxTransform.q.x, pxTransform.q.y, pxTransform.q.z);
		return *this;
	}

	Transform::operator physx::PxTransform() const {
		physx::PxQuat rot;
		rot.x = rotation.x;
		rot.y = rotation.y;
		rot.z = rotation.z;
		rot.w = rotation.w;
		return physx::PxTransform(position.x, position.y, position.z, rot);
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

	sa::Matrix4x4 Transform::getMatrix() const {
		return glm::translate(sa::Matrix4x4(1), position) * glm::toMat4(rotation) * glm::scale(sa::Matrix4x4(1), scale);
	}

	
}
