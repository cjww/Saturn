#pragma once

#define SET_PER_OBJECT 0
#define SET_PER_FRAME 1

#define MAX_VIEWPORT_COUNT 4

namespace sa {
	struct PerObjectBuffer {
		glm::mat4 worldMatrix;
	};

	struct PerFrameBuffer {
		glm::mat4 projViewMatrix;
	};
}
