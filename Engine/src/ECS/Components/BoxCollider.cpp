#include "pch.h"
#include "BoxCollider.h"

#include "ECS/Entity.h"
#include "ECS/Components/RigidBody.h"

namespace comp {
	BoxCollider::BoxCollider(const sa::Vector3& halfLengths)
		: halfLengths(halfLengths)
	{
	}
	BoxCollider::BoxCollider(const sa::Vector3& halfLengths, const sa::Vector3& offset)
		: halfLengths(halfLengths)
		, offset(offset)
	{
	}

	void BoxCollider::serialize(sa::Serializer& s) {
		s.value("halfLengths", (glm::vec3)halfLengths);
		s.value("offset", (glm::vec3)offset);
	}

	void BoxCollider::deserialize(void* pDoc) {
		using namespace simdjson::ondemand;
		object& obj = *(object*)pDoc;
		object member = obj["halfLengths"];
		halfLengths = sa::Serializer::DeserializeVec3(&member);
		member = obj["offset"];
		offset = sa::Serializer::DeserializeVec3(&member);
	}

	void BoxCollider::onConstruct(sa::Entity* e) {
		using namespace physx;
		comp::RigidBody* rb = e->getComponent<comp::RigidBody>();
		if (!rb)
			rb = e->addComponent<comp::RigidBody>();

		PxBoxGeometry box(sa::PhysicsSystem::toPxVec(halfLengths));
		if (offset.x + offset.y + offset.z > 0.01f) {
			pShape = sa::PhysicsSystem::get().createExclusiveShape(&box);
			PxTransform transform(sa::PhysicsSystem::toPxVec(offset));
			pShape->setLocalPose(transform);
		}
		else {
			pShape = sa::PhysicsSystem::get().createShape(&box);
		}

		rb->pActor->attachShape(*pShape);
	}
	
	void BoxCollider::onUpdate(sa::Entity* e) {
		using namespace physx;
		comp::RigidBody* rb = e->getComponent<comp::RigidBody>();
		if (!rb)
			rb = e->addComponent<comp::RigidBody>();
		rb->pActor->detachShape(*pShape);
		
		halfLengths = glm::max(halfLengths, 0.01f);

		PxBoxGeometry box(sa::PhysicsSystem::toPxVec(halfLengths));
		if (offset.x + offset.y + offset.z > 0.01f) {
			pShape = sa::PhysicsSystem::get().createExclusiveShape(&box);
			rb->pActor->attachShape(*pShape);

			PxTransform transform(sa::PhysicsSystem::toPxVec(offset));
			pShape->setLocalPose(transform);
		}
		else {
			pShape = sa::PhysicsSystem::get().createShape(&box);
			rb->pActor->attachShape(*pShape);
		}

	}
	
	void BoxCollider::onDestroy(sa::Entity* e) {
		comp::RigidBody* rb = e->getComponent<comp::RigidBody>();
		if (rb)
			rb->pActor->detachShape(*pShape);
	}

}