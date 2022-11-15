#include "pch.h"
#include "PhysicsSystem.h"

class ErrorCallback : public physx::PxErrorCallback {
	virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override {
		SA_DEBUG_LOG_ERROR(message, "[Code ", (uint32_t)code, "] ", file, ":", line);
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
		using namespace physx;

		SA_DEBUG_LOG_INFO("Initializing PhysX ", PX_PHYSICS_VERSION_MAJOR, ".", PX_PHYSICS_VERSION_MINOR, ".", PX_PHYSICS_VERSION_BUGFIX);
		m_pFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_defaultAllocator, s_defaultErrorCallback);
		if (!m_pFoundation) {
			throw std::runtime_error("PxCreateFoundation failed!");
		}


#ifdef _DEBUG
		constexpr bool trackAllocations = true;

		m_pPvd = PxCreatePvd(*m_pFoundation);
		m_pPvdTransport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
		if (!m_pPvd->connect(*m_pPvdTransport, PxPvdInstrumentationFlag::eALL)) {
			SA_DEBUG_LOG_WARNING("Physx PVD failed to connect");
		}
#else
		constexpr bool trackAllocations = false;
		m_pPvd = nullptr;
		m_pPvdTransport = nullptr;
#endif // _DEBUG

		m_pPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_pFoundation, physx::PxTolerancesScale(), trackAllocations, m_pPvd);
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

#ifdef _DEBUG
		if (m_pPvd)
			m_pPvd->release();
		if (m_pPvdTransport)
			m_pPvdTransport->release();
#endif // _DEBUG

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
		desc.flags = physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;
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

	physx::PxShape* PhysicsSystem::createShape(const physx::PxGeometry* pGeometry, physx::PxMaterial* pMaterial) {
		if (!pMaterial)
			pMaterial = m_pDefaultMaterial;
		size_t hashedValue;
		switch (pGeometry->getType()) {
		case physx::PxGeometryType::eBOX: {
			physx::PxBoxGeometry* pBox = (physx::PxBoxGeometry*)pGeometry;
			std::string str(32, 0);
			sprintf_s(str.data(), str.size(), "Box%.3f%.3f%.3f", pBox->halfExtents.x, pBox->halfExtents.y, pBox->halfExtents.z);
			hashedValue = std::hash<std::string>()(str);
			break;
		}
		case physx::PxGeometryType::eSPHERE: {
			physx::PxSphereGeometry* pSphere = (physx::PxSphereGeometry*)pGeometry;
			std::string str(17, 0);
			sprintf_s(str.data(), str.size(), "Sphere%.3f", pSphere->radius);
			hashedValue = std::hash<std::string>()(str);
			break;
		}
		default:
			break;
		}
		if (m_shapes.count(hashedValue)) {
			return m_shapes.at(hashedValue);
		}
		m_shapes[hashedValue] = m_pPhysics->createShape(*pGeometry, *pMaterial);

		return m_shapes[hashedValue];
	}

	physx::PxShape* PhysicsSystem::createExclusiveShape(const physx::PxGeometry* pGeometry, physx::PxMaterial* pMaterial) {
		if (!pMaterial)
			pMaterial = m_pDefaultMaterial;
		return m_pPhysics->createShape(*pGeometry, *pMaterial, true);
	}

	Vector3 PhysicsSystem::toVector(physx::PxVec3 vec) {
		return Vector3(vec.x, vec.y, vec.z);
	}

	physx::PxVec3 PhysicsSystem::toPxVec(Vector3 vec) {
		return physx::PxVec3(vec.x, vec.y, vec.z);
	}

}
