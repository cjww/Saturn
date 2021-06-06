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
	vr::BufferPtr pUniformBuffer = nullptr;
};