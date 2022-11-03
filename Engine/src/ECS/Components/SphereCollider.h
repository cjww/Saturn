#pragma once
#include "ECS/ComponentBase.h"
#include "PhysicsSystem.h"
namespace comp {

	class SphereCollider : public sa::ComponentBase {
	public:
		float radius = 1.f;
		physx::PxMaterial* pMaterial = nullptr;
		physx::PxShape* pShape = nullptr;

		SphereCollider() = default;
		SphereCollider(float radius);

		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;

		virtual void onConstruct(sa::Entity* e) override;
		virtual void onUpdate(sa::Entity* e) override;
		virtual void onDestroy(sa::Entity* e) override;


	};
}