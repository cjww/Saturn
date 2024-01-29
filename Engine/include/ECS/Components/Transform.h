#pragma once

#include "Tools\Vector.h"
#include "Tools\Matrix.h"

#include "ECS/ComponentBase.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm\gtx/quaternion.hpp"

#include "PhysicsSystem.h"


namespace comp {

	class Transform : public sa::ComponentBase {
	public:
		sa::Vector3 position = sa::Vector3(0);
		glm::quat rotation = glm::quat_identity<float, glm::packed_highp>();
		sa::Vector3 scale = sa::Vector3(1);

		bool hasParent = false;
		sa::Vector3 relativePosition;
		

		Transform() = default;
		Transform(const Transform&) = default;
		Transform(Transform&&) = default;
		Transform& operator=(const Transform&) = default;
		Transform& operator=(Transform&&) noexcept = default;

		Transform(physx::PxTransform pxTransform);
		Transform& operator=(const physx::PxTransform pxTransform);
		operator physx::PxTransform() const;

		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;


		sa::Matrix4x4 getMatrix() const;

	};
}
