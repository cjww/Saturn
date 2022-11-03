#include "pch.h"
#include "BoxCollider.h"

#include "ECS/Entity.h"
#include "ECS/Components/RigidBody.h"

namespace comp {
	BoxCollider::BoxCollider(const sa::Vector3& scale)
		: scale(scale)
	{
	}
	BoxCollider::BoxCollider(const sa::Vector3& scale, const sa::Vector3& offset)
		: scale(scale)
		, offset(offset)
	{
	}

	void BoxCollider::serialize(sa::Serializer& s) {

	}

	void BoxCollider::deserialize(void* pDoc) {
	
	}

	void BoxCollider::onConstruct(sa::Entity* e) {
		comp::RigidBody* rb = e->getComponent<comp::RigidBody>();
		if (!rb)
			rb = e->addComponent<comp::RigidBody>();

		pShape = sa::PhysicsSystem::get().createBox(scale);
		rb->pActor->attachShape(*pShape);
	}
	
	void BoxCollider::onUpdate(sa::Entity* e) {
		pShape->getGeometry().box().halfExtents = sa::PhysicsSystem::toPxVec(scale);
	}
	
	void BoxCollider::onDestroy(sa::Entity* e) {
		comp::RigidBody* rb = e->getComponent<comp::RigidBody>();
		if (rb)
			rb->pActor->detachShape(*pShape);
		pShape->release();
	}

}