#pragma once
#include "ECS/ComponentBase.h"
#include <Tools\Vector.h>

#include "PhysicsSystem.h"

namespace comp {

	class BoxCollider : public sa::ComponentBase {
	public:
		sa::Vector3 scale = sa::Vector3(1.0f);
		sa::Vector3 offset = sa::Vector3(0.0f);

		physx::PxMaterial* pMaterial = nullptr;
		physx::PxShape* pShape = nullptr;

		BoxCollider() = default;
		BoxCollider(const sa::Vector3& scale);
		BoxCollider(const sa::Vector3& scale, const sa::Vector3& offset);

		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;

		virtual void onConstruct(sa::Entity* e) override;
		virtual void onUpdate(sa::Entity* e) override;
		virtual void onDestroy(sa::Entity* e) override;

	};
}

