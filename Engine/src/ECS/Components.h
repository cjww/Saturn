#pragma once
#include "ECS/Components/Name.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/Light.h"
#include "ECS/Components/Model.h"

#include "PhysicsSystem.h"

namespace sa {
	void registerAllComponents();
}

namespace comp {
	struct Script /*: public sa::LuaAccessable */ {
		sol::environment env;
		/*
		static void reg() {
			auto type = registerType<Script>();
			type["__index"] = [](const comp::Script& script, const std::string& key) {
				return script.env[key];
			};
			type["__newindex"] = [](comp::Script& script, const std::string& key, const sol::lua_value& value) {
				script.env[key] = value;
			};

		}
		*/
	};

	struct RigidBody : public sa::ComponentBase {
		physx::PxRigidActor* pActor = nullptr;
		bool isStatic = true;

		RigidBody() = default;
		RigidBody(bool isStatic) 
			: isStatic(isStatic)
		{
		}

		virtual void serialize(sa::Serializer& s) override {}
		virtual void deserialize(void* pDoc) override {}

	};

	struct SphereCollider : public sa::ComponentBase {
		float radius = 1.f;
		physx::PxMaterial* pMaterial = nullptr;
		physx::PxShape* pShape = nullptr;

		SphereCollider() = default;
		SphereCollider(float radius) 
			: radius(radius)
		{
		}

		virtual void serialize(sa::Serializer& s) override {}
		virtual void deserialize(void* pDoc) override {}
	};


	struct BoxCollider : public sa::ComponentBase {
		sa::Vector3 halfLengths = sa::Vector3(1);
		physx::PxMaterial* pMaterial = nullptr;
		physx::PxShape* pShape = nullptr;

		BoxCollider() = default;
		BoxCollider(const sa::Vector3& halfLengths)
			: halfLengths(halfLengths)
		{
		}

		virtual void serialize(sa::Serializer& s) override {}
		virtual void deserialize(void* pDoc) override {}
	};

};

