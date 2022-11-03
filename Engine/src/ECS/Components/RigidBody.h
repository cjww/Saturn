#pragma once
#include "ECS/ComponentBase.h"
#include "PhysicsSystem.h"

namespace comp {
	class RigidBody : public sa::ComponentBase {
	public:
		physx::PxRigidActor* pActor = nullptr;
		bool isStatic = true;

		RigidBody() = default;
		RigidBody(bool isStatic);

		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;

		virtual void onConstruct(sa::Entity* e) override;
		virtual void onUpdate(sa::Entity* e) override;
		virtual void onDestroy(sa::Entity* e) override;

	};
}