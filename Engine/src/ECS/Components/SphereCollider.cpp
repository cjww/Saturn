#include "pch.h"
#include "SphereCollider.h"
#include "ECS/Entity.h"
#include "ECS/Components/RigidBody.h"

namespace comp {
	SphereCollider::SphereCollider(float radius)
		: radius(radius)
	{
	}


	void SphereCollider::serialize(sa::Serializer& s) {
	
	}
	
	void SphereCollider::deserialize(void* pDoc) {

	}


	void SphereCollider::onConstruct(sa::Entity* e) {
		using namespace physx;
		comp::RigidBody* rb = e->getComponent<comp::RigidBody>();
		if (!rb)
			rb = e->addComponent<comp::RigidBody>();

		PxSphereGeometry sphere(radius);
		if (offset.x + offset.y + offset.z > 0.01f) {
			pShape = sa::PhysicsSystem::get().createExclusiveShape(&sphere);
			PxTransform transform(sa::PhysicsSystem::toPxVec(offset));
			pShape->setLocalPose(transform);
		}
		else {
			pShape = sa::PhysicsSystem::get().createShape(&sphere);
		}

		rb->pActor->attachShape(*pShape);
	}
	
	void SphereCollider::onUpdate(sa::Entity* e) {
		using namespace physx;
		comp::RigidBody* rb = e->getComponent<comp::RigidBody>();
		if (!rb)
			rb = e->addComponent<comp::RigidBody>();
		rb->pActor->detachShape(*pShape);
		if (radius < 0.01f) radius = 0.01f;

		PxSphereGeometry sphere(radius);
		if (offset.x + offset.y + offset.z > 0.01f) {
			pShape = sa::PhysicsSystem::get().createExclusiveShape(&sphere);
			PxTransform transform(sa::PhysicsSystem::toPxVec(offset));
			pShape->setLocalPose(transform);
		}
		else {
			pShape = sa::PhysicsSystem::get().createShape(&sphere);
		}

		rb->pActor->attachShape(*pShape);
	}
	
	void SphereCollider::onDestroy(sa::Entity* e) {
		using namespace physx;
		comp::RigidBody* rb = e->getComponent<comp::RigidBody>();
		if (rb) 
			rb->pActor->detachShape(*pShape);
	}


}