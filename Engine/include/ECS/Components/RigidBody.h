#pragma once
#include "ECS/ComponentBase.h"
#include "PhysicsSystem.h"

#include "ECS/Components/Transform.h"

namespace comp {
	class RigidBody : public sa::ComponentBase {
	private:
		friend class SphereCollider;
		friend class BoxCollider;

		physx::PxRigidActor* m_pActor = nullptr;
		bool m_isStatic = true;
	public:

		RigidBody() = default;
		RigidBody(const RigidBody&) = default;
		RigidBody(RigidBody&&) = default;
		RigidBody& operator=(const RigidBody& other);
		RigidBody& operator=(RigidBody&&) noexcept = default;
		

		RigidBody(bool isStatic);

		void setMass(float mass);
		float getMass() const;

		void setKinematic(bool isKinematic);
		bool isKinematic() const;

		void setGravityEnabled(bool isGravityEnabled);
		bool isGravityEnabled() const;

		void setStatic(bool isStatic);
		bool isStatic() const;

		void setGlobalPose(const comp::Transform& transform);


		virtual void serialize(sa::Serializer& s) override;
		virtual void deserialize(void* pDoc) override;

		virtual void onConstruct(sa::Entity* e) override;
		virtual void onUpdate(sa::Entity* e) override;
		virtual void onDestroy(sa::Entity* e) override;


	};
}