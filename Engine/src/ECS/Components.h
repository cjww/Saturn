#pragma once
#include <Resources/Buffer.hpp>
#include "AssetManager.h"
#include "Tools\Vector.h"

#include "sol\environment.hpp"

#include "ComponentBase.h"

#include "Entity.h"


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
		sa::Vector3 rotation = sa::Vector3(0);
		sa::Vector3 scale = sa::Vector3(1);

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

	enum class LightType {
		POINT = 0,
		DIRECTIONAL = 1
	};
	struct Light {
		glm::vec3 color = glm::vec3(1);
		float strength = 1.0f;
		LightType type = LightType::POINT;
	};

};

