#pragma once
#include <ResourceManager.h>
#include "Tools\Vector.h"

namespace comp {

	struct Name {
		std::string name;
	};

	struct Model {
		ResourceID modelID = NULL_RESOURCE;
		vr::Buffer* buffer = nullptr;
		vr::DescriptorSetPtr descriptorSet = nullptr;
	};

	struct Transform {
		sa::Vector3f position = sa::Vector3f(0);
		sa::Vector3f rotation = sa::Vector3f(0);
		sa::Vector3f scale = sa::Vector3f(1);
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

	struct Script {
		// TODO not use dynamic memory here
		std::string scriptName;
		std::string code;
	};

	
};

