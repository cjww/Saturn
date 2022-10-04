#include "pch.h"
#include "Components.h"

#include "AssetManager.h"

#include <PxPhysicsAPI.h>

namespace comp {
	Model::Model(const std::string& name) {
		sa::AssetManager::get().loadModel(name);
	}

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

	sa::Matrix4x4 Transform::getMatrix() const {
		return glm::translate(sa::Matrix4x4(1), position) * glm::toMat4(rotation) * glm::scale(sa::Matrix4x4(1), scale);
	}
}
