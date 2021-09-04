#pragma once
#include <ResourceManager.h>

#include <glm/gtc/matrix_transform.hpp>

struct Model {
	ResourceID modelID = NULL_RESOURCE;
};

struct Transform {
	glm::vec3 position = glm::vec3(0);
	glm::vec3 rotation = glm::vec3(0);
	glm::vec3 scale = glm::vec3(1);
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
	std::string scriptName;
	std::string code;
};
