#pragma once
#include <ResourceManager.h>

namespace sa {

	struct VertexUV {
		glm::vec4 position;
		glm::vec2 texCoord;
	};

	struct VertexColor {
		glm::vec4 position;
		glm::vec4 color;
	};
}
