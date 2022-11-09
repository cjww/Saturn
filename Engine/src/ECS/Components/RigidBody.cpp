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

	RigidBody& RigidBody::operator=(const RigidBody& other) {
		
		// copy actor
		sa::Entity* userData = nullptr;
		if (other.isStatic != isStatic) {
			pActor->getScene()->removeActor(*pActor);
			userData = (sa::Entity*)pActor->userData;
			pActor->release();
			pActor = nullptr;
			
			
			pActor = sa::PhysicsSystem::get().createRigidBody(other.isStatic, other.pActor->getGlobalPose());
			
			pActor->userData = userData;
			other.pActor->getScene()->addActor(*pActor);
		}
		else {
			pActor->setGlobalPose(other.pActor->getGlobalPose());
		}

		std::vector<physx::PxShape*> oldShapes(pActor->getNbShapes());
		pActor->getShapes(oldShapes.data(), oldShapes.size());
		for (const auto& pShape : oldShapes) {
			pActor->detachShape(*pShape);
		}
		std::vector<physx::PxShape*> shapes(other.pActor->getNbShapes());
		other.pActor->getShapes(shapes.data(), shapes.size());
		for (const auto& pShape : shapes) {
			pActor->attachShape(*pShape);
		}

		isStatic = other.isStatic;

		return *this;
	}
}