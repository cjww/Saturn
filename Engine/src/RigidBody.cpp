#include "pch.h"
#include "ECS/Components/RigidBody.h"

#include "ECS/Entity.h"
#include "Scene.h"
namespace comp {
	RigidBody::RigidBody(bool isStatic)
		: m_isStatic(isStatic)
	{
	}

	RigidBody& RigidBody::operator=(const RigidBody& other) {
		// copy actor
		sa::Entity* userData = nullptr;
		if (other.isStatic() != isStatic()) {
			m_pActor->getScene()->removeActor(*m_pActor);
			userData = (sa::Entity*)m_pActor->userData;
			m_pActor->release();
			m_pActor = nullptr;


			m_pActor = sa::PhysicsSystem::get().createRigidBody(other.m_isStatic, other.m_pActor->getGlobalPose());

			m_pActor->userData = userData;
			other.m_pActor->getScene()->addActor(*m_pActor);
		}
		else {
			m_pActor->setGlobalPose(other.m_pActor->getGlobalPose());
		}

		std::vector<physx::PxShape*> oldShapes(m_pActor->getNbShapes());
		m_pActor->getShapes(oldShapes.data(), oldShapes.size());
		for (const auto& pShape : oldShapes) {
			m_pActor->detachShape(*pShape);
		}
		std::vector<physx::PxShape*> shapes(other.m_pActor->getNbShapes());
		other.m_pActor->getShapes(shapes.data(), shapes.size());
		for (const auto& pShape : shapes) {
			m_pActor->attachShape(*pShape);
		}

		m_isStatic = other.m_isStatic;

		return *this;
	}

	void RigidBody::setMass(float mass) {
		auto pDynamic = m_pActor->is<physx::PxRigidDynamic>();
		if (pDynamic) {
			pDynamic->setMass(mass);
		}
	}

	float RigidBody::getMass() const {
		auto pDynamic = m_pActor->is<physx::PxRigidDynamic>();
		if (pDynamic) {
			return pDynamic->getMass();
		}
		return 0.0f;
	}

	void RigidBody::setKinematic(bool isKinematic) {
		auto pDynamic = m_pActor->is<physx::PxRigidBody>();
		if (pDynamic) {
			pDynamic->setRigidBodyFlag(physx::PxRigidBodyFlag::Enum::eKINEMATIC, isKinematic);
		}
	}

	bool RigidBody::isKinematic() const {
		auto pDynamic = m_pActor->is<physx::PxRigidBody>();
		if (pDynamic) {
			return (pDynamic->getRigidBodyFlags() & physx::PxRigidBodyFlag::Enum::eKINEMATIC) == physx::PxRigidBodyFlag::Enum::eKINEMATIC;
		}
		return false;
	}

	void RigidBody::setGravityEnabled(bool isGravityEnabled) {
		m_pActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, !isGravityEnabled);
	}

	bool RigidBody::isGravityEnabled() const {
		return (m_pActor->getActorFlags() & physx::PxActorFlag::eDISABLE_GRAVITY) != physx::PxActorFlag::eDISABLE_GRAVITY;
	}

	void RigidBody::setStatic(bool isStatic) {
		if (m_isStatic == isStatic)
			return;

		m_isStatic = isStatic;
		float oldMass = getMass();

		std::vector<physx::PxShape*> oldShapes;
		physx::PxRigidActor* pOldActor = m_pActor;

		uint32_t shapeCount = m_pActor->getNbShapes();
		oldShapes.resize(shapeCount);
		m_pActor->getShapes(oldShapes.data(), oldShapes.size());
		

		sa::Entity* e = static_cast<sa::Entity*>(m_pActor->userData);

		comp::Transform* transform = e->getComponent<comp::Transform>();
		if (!transform)
			transform = e->addComponent<comp::Transform>();

		m_pActor = sa::PhysicsSystem::get().createRigidBody(m_isStatic, *transform);
		m_pActor->userData = pOldActor->userData;

		for (const auto& pShape : oldShapes) {
			m_pActor->attachShape(*pShape);
		}

		if (pOldActor) {
			e->getScene()->m_pPhysicsScene->removeActor(*pOldActor, false);
			pOldActor->release();
		}
		e->getScene()->m_pPhysicsScene->addActor(*m_pActor);
	
		setMass(oldMass);
	}

	bool RigidBody::isStatic() const {
		return m_isStatic;
	}

	void RigidBody::setGlobalPose(const comp::Transform& transform) {
		if (isKinematic()) {
			auto pDynamic = m_pActor->is<physx::PxRigidDynamic>();
			if (pDynamic) {
				pDynamic->setKinematicTarget(transform);
			}
			return;
		}
		m_pActor->setGlobalPose(transform, false);
	}

	void RigidBody::serialize(sa::Serializer& s) {
		s.value("isStatic", isStatic());
		s.value("mass", getMass());
		s.value("isKinematic", isKinematic());
		s.value("isGravityEnabled", isGravityEnabled());

	}
	
	void RigidBody::deserialize(void* pDoc) {
		simdjson::ondemand::object& obj = *(simdjson::ondemand::object*)pDoc;
		bool isStatic = obj["isStatic"];
		setStatic(isStatic);
		double mass = obj["mass"];
		setMass(mass);
	}

	void RigidBody::onConstruct(sa::Entity* e) {
		comp::Transform* transform = e->getComponent<comp::Transform>();
		if (!transform)
			transform = e->addComponent<comp::Transform>();
		m_pActor = sa::PhysicsSystem::get().createRigidBody(m_isStatic, *transform);
		if (!m_pActor) {
			SA_DEBUG_LOG_ERROR("Failed to create rigidbody actor");
			return;
		}
		m_pActor->userData = new sa::Entity(*e);
		e->getScene()->m_pPhysicsScene->addActor(*m_pActor);
	}

	void RigidBody::onUpdate(sa::Entity* e) {
		auto pDynamic = m_pActor->is<physx::PxRigidDynamic>();

		if (pDynamic) {
			pDynamic->setLinearVelocity({ 0, 0, 0 }, false);
			pDynamic->setAngularVelocity({ 0, 0, 0 }, false);
		}
		return;
		std::vector<physx::PxShape*> oldShapes;
		physx::PxRigidActor* pOldActor = nullptr;

		if (m_pActor) {
			pOldActor = m_pActor;
			uint32_t shapeCount = m_pActor->getNbShapes();
			oldShapes.resize(shapeCount);
			m_pActor->getShapes(oldShapes.data(), oldShapes.size());
		}
		
		comp::Transform* transform = e->getComponent<comp::Transform>();
		if (!transform) 
			transform = e->addComponent<comp::Transform>();

		m_pActor = sa::PhysicsSystem::get().createRigidBody(m_isStatic, *transform);
		m_pActor->userData = new sa::Entity(*e);
		
		for (const auto& pShape : oldShapes) {
			m_pActor->attachShape(*pShape);
		}

		if (pOldActor) {
			delete pOldActor->userData;
			e->getScene()->m_pPhysicsScene->removeActor(*pOldActor, false);
			pOldActor->release();
		}
		e->getScene()->m_pPhysicsScene->addActor(*m_pActor);
	}

	void RigidBody::onDestroy(sa::Entity* e) {
		delete m_pActor->userData;
		e->getScene()->m_pPhysicsScene->removeActor(*m_pActor);
		m_pActor->release();
	}

}