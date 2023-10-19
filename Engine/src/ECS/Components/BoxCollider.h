#pragma once
#include "ECS/ComponentBase.h"
#include <Tools\Vector.h>

#include "PhysicsSystem.h"

namespace comp {

	class BoxCollider : public sa::ComponentBase {
	public:

		sa::Vector3 halfLengths = sa::Vector3(1.0f);
		sa::Vector3 offset = sa::Vector3(0.0f);

		physx::PxMaterial* pMaterial = nullptr;
		physx::PxShape* pShape = nullptr;

		BoxCollider() = default;
		BoxCollider(const BoxCollider&) = default;
		BoxCollider(BoxCollider&&) = default;
		BoxCollider& operator=(const BoxCollider&) = default;
		BoxCollider& operator=(BoxCollider&&) = default;

		BoxCollider(const sa::Vector3& halfLengths);
		BoxCollider(const sa::Vector3& halfLengths, const sa::Vector3& offset);

		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;

		virtual void onConstruct(sa::Entity* e) override;
		virtual void onUpdate(sa::Entity* e) override;
		virtual void onDestroy(sa::Entity* e) override;

	};
}

