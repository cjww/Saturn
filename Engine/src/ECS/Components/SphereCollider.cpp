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
		s.value("radius", radius);
		s.value("offset", (glm::vec3)offset);
	}
	
	void SphereCollider::deserialize(void* pDoc) {
		using namespace simdjson::ondemand;
		object& obj = *(object*)pDoc;
		radius = (float)obj["radius"].get_double();
		object member = obj["offset"];
		offset = sa::Serializer::DeserializeVec3(&member);
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

		rb->m_pActor->attachShape(*pShape);
	}
	
	void SphereCollider::onUpdate(sa::Entity* e) {
		using namespace physx;
		comp::RigidBody* rb = e->getComponent<comp::RigidBody>();
		if (!rb)
			rb = e->addComponent<comp::RigidBody>();
		rb->m_pActor->detachShape(*pShape);

		radius = std::max(radius, 0.01f);

		PxSphereGeometry sphere(radius);
		if (offset.x + offset.y + offset.z > 0.01f) {
			pShape = sa::PhysicsSystem::get().createExclusiveShape(&sphere);
			PxTransform transform(sa::PhysicsSystem::toPxVec(offset));
			pShape->setLocalPose(transform);
		}
		else {
			pShape = sa::PhysicsSystem::get().createShape(&sphere);
		}

		rb->m_pActor->attachShape(*pShape);
	}
	
	void SphereCollider::onDestroy(sa::Entity* e) {
		using namespace physx;
		comp::RigidBody* rb = e->getComponent<comp::RigidBody>();
		if (rb) 
			rb->m_pActor->detachShape(*pShape);
	}


}