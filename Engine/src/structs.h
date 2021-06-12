#pragma once
#include <glm/mat4x4.hpp>

#define PER_FRAME_SET 0
#define PER_OBJECT_SET 1

struct PerObjectBuffer {
	glm::mat4 worldMatrix;
};
