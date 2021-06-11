#pragma once
#include <glm/mat4x4.hpp>

#define PER_FRAME_SET 1
#define PER_OBJECT_SET 0

struct PerObjectBuffer {
	glm::mat4 worldMatrix;
};
