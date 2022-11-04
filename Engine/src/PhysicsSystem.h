#pragma once

#include <Tools/Vector.h>

namespace physx {
	class PxFoundation;
	class PxPhysics;
	class PxCpuDispatcher;
	class PxDefaultCpuDispatcher;


	class PxScene;
	class PxRigidBody;
	class PxRigidActor;
	class PxMaterial;
	class PxShape;
	class PxGeometry;
	class PxTransform;
	class PxVec3;

	class PxPvd;
	class PxPvdTransport;
}

namespace sa {
	class PhysicsSystem {
	private:
		physx::PxFoundation* m_pFoundation;
		physx::PxPhysics* m_pPhysics;
		physx::PxDefaultCpuDispatcher* m_pCpuDispatcher;

		physx::PxPvd* m_pPvd;
		physx::PxPvdTransport* m_pPvdTransport;

		physx::PxMaterial* m_pDefaultMaterial;

		std::unordered_map<size_t, physx::PxShape*> m_shapes;

		PhysicsSystem();
	public:

		~PhysicsSystem();

		static PhysicsSystem& get();

		physx::PxScene* createScene();

		physx::PxRigidActor* createRigidBody(bool isStatic, physx::PxTransform transform);
		physx::PxMaterial* createMaterial(float staticFriction, float dynamicFriction, float restitution);

		physx::PxShape* createShape(const physx::PxGeometry* pGeometry, physx::PxMaterial* pMaterial = nullptr);
		physx::PxShape* createExclusiveShape(const physx::PxGeometry* pGeometry, physx::PxMaterial* pMaterial = nullptr);

		static Vector3 toVector(physx::PxVec3 vec);
		static physx::PxVec3 toPxVec(Vector3 vec);

	};


}