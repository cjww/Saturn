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
		comp::RigidBody* rb = e->getComponent<comp::RigidBody>();
		if (!rb)
			rb = e->addComponent<comp::RigidBody>();

		pShape = sa::PhysicsSystem::get().createSphere(radius);

		rb->pActor->attachShape(*pShape);
	}
	
	void SphereCollider::onUpdate(sa::Entity* e) {
		pShape->getGeometry().sphere().radius = radius;
	}
	
	void SphereCollider::onDestroy(sa::Entity* e) {
		comp::RigidBody* rb = e->getComponent<comp::RigidBody>();
		if (rb)
			rb->pActor->detachShape(*pShape);
		pShape->release();
	}


}