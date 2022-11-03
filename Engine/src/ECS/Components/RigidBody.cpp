#include "pch.h"
#include "RigidBody.h"

#include "ECS/Entity.h"
#include "Scene.h"
namespace comp {
	RigidBody::RigidBody(bool isStatic)
		: isStatic(isStatic)
	{
	}

	void RigidBody::serialize(sa::Serializer& s) {
		s.value("isStatic", isStatic);
	}
	
	void RigidBody::deserialize(void* pDoc) {
		simdjson::ondemand::object& obj = *(simdjson::ondemand::object*)pDoc;
		isStatic = obj["isStatic"];
		
	}

	void RigidBody::onConstruct(sa::Entity* e) {
		comp::Transform* transform = e->getComponent<comp::Transform>();
		if (!transform)
			transform = e->addComponent<comp::Transform>();
		pActor = sa::PhysicsSystem::get().createRigidBody(isStatic, *transform);
		pActor->userData = new sa::Entity(*e);
		e->getScene()->m_pPhysicsScene->addActor(*pActor);
	}

	void RigidBody::onUpdate(sa::Entity* e) {
		std::vector<physx::PxShape*> oldShapes;
		physx::PxRigidActor* pOldActor = nullptr;

		if (pActor) {
			pOldActor = pActor;
			uint32_t shapeCount = pActor->getNbShapes();
			oldShapes.resize(shapeCount);
			pActor->getShapes(oldShapes.data(), oldShapes.size());
		}
		
		comp::Transform* transform = e->getComponent<comp::Transform>();
		if (!transform) 
			transform = e->addComponent<comp::Transform>();

		pActor = sa::PhysicsSystem::get().createRigidBody(isStatic, *transform);
		pActor->userData = new sa::Entity(*e);
		
		for (const auto& pShape : oldShapes) {
			pActor->attachShape(*pShape);
		}

		if (pOldActor) {
			delete pOldActor->userData;
			e->getScene()->m_pPhysicsScene->removeActor(*pOldActor, false);
			pOldActor->release();
		}
		e->getScene()->m_pPhysicsScene->addActor(*pActor);
	}

	void RigidBody::onDestroy(sa::Entity* e) {
		delete pActor->userData;
		e->getScene()->m_pPhysicsScene->removeActor(*pActor);
		pActor->release();
	}


}