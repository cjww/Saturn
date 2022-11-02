#pragma once

#include "Tools\Vector.h"
#include "Tools\Matrix.h"

#include "ECS/ComponentBase.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm\gtx/quaternion.hpp"

#include "PhysicsSystem.h"


namespace comp {

	struct Transform : public sa::LuaAccessable {
		sa::Vector3 position = sa::Vector3(0);
		glm::quat rotation = glm::quat_identity<float, glm::packed_highp>();
		sa::Vector3 scale = sa::Vector3(1);

		bool hasParent = false;
		sa::Vector3 relativePosition;
		

		Transform() = default;
		Transform(physx::PxTransform pxTransform);
		comp::Transform& operator=(const physx::PxTransform pxTransform);
		operator physx::PxTransform() const;

		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;


		sa::Matrix4x4 getMatrix() const;

		static void reg();
	};
}