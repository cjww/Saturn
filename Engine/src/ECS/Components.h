#pragma once
#include <Resources/Buffer.hpp>
#include "AssetManager.h"
#include "Tools\Vector.h"

#include "sol\environment.hpp"

#include "ComponentBase.h"

#include "Entity.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm\gtx/quaternion.hpp"

#include "PhysicsSystem.h"

namespace sa {

	enum class LightType : uint32_t {
		POINT = 0,
		DIRECTIONAL = 1,
		SPOT = 2,
	};

	struct alignas(16) LightData {
		Color color = SA_COLOR_WHITE;	// 16 bytes
		sa::Vector3 position = sa::Vector3(0);		// 12 bytes - 16 offset
		float intensity = 1.0f;						// 4 bytes - 28 offset
		float attenuationRadius = 7.0f;
		LightType type = LightType::POINT;
	};

}


namespace comp {

	struct Name : public sa::ComponentBase {
		std::string name;
		
		Name() {}
		Name(std::string name) : name(name) {}

	};

	struct Model : public sa::LuaAccessable {
		ResourceID modelID = NULL_RESOURCE;

		Model() = default;
		Model(const std::string& name);
		Model(ResourceID id);


		static void reg() {
			auto type = registerType<Model>("",
				sol::constructors<Model(), Model(const std::string&)>()
				);

			type["id"] = &comp::Model::modelID;
		}

	};

	struct Transform : public sa::LuaAccessable {
		sa::Vector3 position = sa::Vector3(0);
		glm::quat rotation = glm::quat_identity<float, glm::packed_highp>();
		sa::Vector3 scale = sa::Vector3(1);

		bool hasParent = false;
		sa::Vector3 relativePosition;
		
		Transform() = default;
		Transform(physx::PxTransform pxTransform);

		comp::Transform& operator=(const physx::PxTransform pxTransform);

		operator physx::PxTransform() const;

		sa::Matrix4x4 getMatrix() const;

		static void reg() {
			auto type = registerType<Transform>();
			type["position"] = sol::property(
				[](Transform& self) -> sa::Vector3& {
					if (self.hasParent)
						return self.relativePosition;
					return self.position;
				},
				[](Transform& self, const sa::Vector3& value) {
					if (self.hasParent) {
						self.relativePosition = value;
						return;
					}
					self.position = value;
				}
			);
			type["rotation"] = &Transform::rotation;
			type["scale"] = &Transform::scale;
		}
	};

	struct Script : public sa::LuaAccessable {
		sol::environment env;

		static void reg() {
			auto type = registerType<Script>();
			type["__index"] = [](const comp::Script& script, const std::string& key) {
				return script.env[key];
			};
			type["__newindex"] = [](comp::Script& script, const std::string& key, const sol::lua_value& value) {
				script.env[key] = value;
			};

		}
	};

	struct Light : public sa::LuaAccessable {
		unsigned int index;
		sa::LightData values;

		Light() = default;

		static void reg() {
			auto type = registerType<Light>("", 
				sol::constructors<Light()>()
			);
			type["intensity"] = sol::property([](const Light& self) {return self.values.intensity; }, [](Light& self, float value) { self.values.intensity = value; });
			type["attenuationRadius"] = sol::property([](const Light& self) {return self.values.attenuationRadius; }, [](Light& self, float value) { self.values.attenuationRadius = value; });
			type["color"] = sol::property(
				[](const comp::Light& light) {return sa::Vector4(light.values.color.r, light.values.color.g, light.values.color.b, light.values.color.a); },
				[](comp::Light& self, const sa::Vector4& color) {self.values.color = sa::Color{ color.x, color.y, color.z, color.w }; }
			);
			
		}
	};

	struct RigidBody : public sa::ComponentBase {
		physx::PxRigidActor* pActor = nullptr;
		bool isStatic = true;

		RigidBody() = default;
		RigidBody(bool isStatic) 
			: isStatic(isStatic)
		{
		};

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
	};

};

