#pragma once
#include <Resources/Buffer.hpp>
#include "AssetManager.h"
#include "Tools\Vector.h"

#include "sol\environment.hpp"

#include "ComponentBase.h"

#include "Entity.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm\gtx/quaternion.hpp"

namespace sa {

	enum class LightType : uint32_t {
		POINT = 0,
		DIRECTIONAL = 1
	};

	struct alignas(16) LightData {
		Color color = SA_COLOR_WHITE;	// 16 bytes
		sa::Vector3 position = sa::Vector3(0);		// 12 bytes - 16 offset
		float strength = 1.0f;						// 4 bytes - 28 offset
		LightType type = LightType::POINT;			// 4 bytes - 32 offset - total 36 bytes
	};

}


namespace comp {

	struct Name : public sa::ComponentBase {
		//std::string name;
		std::string name;
		
		Name() {}
		Name(std::string name) : name(name) {}

	};

	struct Model : public sa::LuaAccessable {
		ResourceID modelID = NULL_RESOURCE;
		sa::Buffer buffer;
		ResourceID descriptorSet = NULL_RESOURCE;

		static void luaReg(sol::usertype<comp::Model>& type) {
			type["id"] = &comp::Model::modelID;
		}

	};

	struct Transform : public sa::LuaAccessable {
		sa::Vector3 position = sa::Vector3(0);
		glm::quat rotation = glm::quat_identity<float, glm::packed_highp>();
		sa::Vector3 scale = sa::Vector3(1);


		sa::Matrix4x4 getMatrix() const {
			return glm::translate(sa::Matrix4x4(1), position) * glm::toMat4(rotation) * glm::scale(sa::Matrix4x4(1), scale);
		}

		static void luaReg(sol::usertype<comp::Transform>& type) {
			type["position"] = &comp::Transform::position;
			type["rotation"] = &comp::Transform::rotation;
			type["scale"] = &comp::Transform::scale;
		}
	};

	struct Script : public sa::LuaAccessable {
		sol::environment env;

		static void luaReg(sol::usertype<comp::Script>& type) {
			type["__index"] = [](const comp::Script& script, const std::string& key) {
				return script.env[key];
			};
			type["__newindex"] = [](comp::Script& script, const std::string& key, const sol::lua_value& value) {
				script.env[key] = value;
			};

		}
	};

	struct Light {
		unsigned int index;
		sa::LightData values;
	};

};

