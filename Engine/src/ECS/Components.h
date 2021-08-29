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

	glm::mat4 getMatrix() {
		glm::mat4 mat(1);
		mat = glm::translate(mat, position);
		mat = glm::rotate(mat, rotation.x, glm::vec3(1, 0, 0));
		mat = glm::rotate(mat, rotation.y, glm::vec3(0, 1, 0));
		mat = glm::rotate(mat, rotation.z, glm::vec3(0, 0, 1));
		mat = glm::scale(mat, scale);
		return mat;
	}
};