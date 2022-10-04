#include "pch.h"
#include "PhysicsSystem.h"

#include <PxPhysicsAPI.h>


class ErrorCallback : public physx::PxErrorCallback {
	virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override {
		SA_DEBUG_LOG_ERROR(message, "[Code", (uint32_t)code, "]", file, line);
	}

};

class AllocatorCallback : public physx::PxAllocatorCallback {
	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line) override {
		return malloc(size);
	}
	virtual void deallocate(void* ptr) override {
		free(ptr);
	}

};

namespace sa {
	PhysicsSystem::PhysicsSystem() {
		static AllocatorCallback s_defaultAllocator;
		static ErrorCallback s_defaultErrorCallback;

		m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_defaultAllocator, s_defaultErrorCallback);
		if (!m_pFoundation) {
			throw std::runtime_error("PxCreateFoundation failed!");
		}

		m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, physx::PxTolerancesScale());
		if (!m_pPhysics) {
			throw std::runtime_error("PxCreatePhysics failed!");
		}

		m_pCpuDispatcher = physx::PxDefaultCpuDispatcherCreate(4);


		m_pDefaultMaterial = m_pPhysics->createMaterial(0.5f, 0.5f, 0.1f);
	}

	PhysicsSystem::~PhysicsSystem() {
		m_pDefaultMaterial->release();
		m_pCpuDispatcher->release();
		m_pPhysics->release();
		m_pFoundation->release();
	}

	PhysicsSystem& PhysicsSystem::get() {
		static PhysicsSystem instance;
		return instance;
	}

	physx::PxScene* PhysicsSystem::createScene() {
		physx::PxSceneDesc desc(m_pPhysics->getTolerancesScale());
		desc.gravity = physx::PxVec3(0.0f, -9.82f, 0.0f);
		desc.cpuDispatcher = m_pCpuDispatcher;
		desc.filterShader = physx::PxDefaultSimulationFilterShader;
		return m_pPhysics->createScene(desc);
	}
	physx::PxRigidActor* PhysicsSystem::createRigidBody(bool isSatic, physx::PxTransform transform) {
		if (isSatic)
			return m_pPhysics->createRigidStatic(transform);
		return m_pPhysics->createRigidDynamic(transform);
	}
	
	physx::PxMaterial* PhysicsSystem::createMaterial(float staticFriction, float dynamicFriction, float restitution) {
		return m_pPhysics->createMaterial(staticFriction, dynamicFriction, restitution);
	}

	physx::PxShape* PhysicsSystem::createSphere(float radius, physx::PxMaterial* pMaterial) {
		if (!pMaterial)
			pMaterial = m_pDefaultMaterial;
		return m_pPhysics->createShape(physx::PxSphereGeometry(radius), *pMaterial);
	}

	physx::PxShape* PhysicsSystem::createBox(Vector3 halfLengths, physx::PxMaterial* pMaterial) {
		if (!pMaterial)
			pMaterial = m_pDefaultMaterial;
		return m_pPhysics->createShape(physx::PxBoxGeometry(toPxVec(halfLengths)), *pMaterial);
	}

	Vector3 PhysicsSystem::toVector(physx::PxVec3 vec) {
		return Vector3(vec.x, vec.y, vec.z);
	}

	physx::PxVec3 PhysicsSystem::toPxVec(Vector3 vec) {
		return physx::PxVec3(vec.x, vec.y, vec.z);
	}

}
