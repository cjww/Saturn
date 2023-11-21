#pragma once
#include "ECS/ComponentBase.h"
#include "PhysicsSystem.h"
namespace comp {

	class SphereCollider : public sa::ComponentBase {
	public:
		float radius = 1.f;
		glm::vec3 offset = glm::vec3(0);
		physx::PxMaterial* pMaterial = nullptr;
		physx::PxShape* pShape = nullptr;

		SphereCollider() = default;
		SphereCollider(const SphereCollider&) = default;
		SphereCollider(SphereCollider&&) = default;
		SphereCollider& operator=(const SphereCollider&) = default;
		SphereCollider& operator=(SphereCollider&&) noexcept = default;

		SphereCollider(float radius);

		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;

		virtual void onConstruct(sa::Entity* e) override;
		virtual void onUpdate(sa::Entity* e) override;
		virtual void onDestroy(sa::Entity* e) override;


	};
}