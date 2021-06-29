#pragma once
#include "glm/gtc/matrix_transform.hpp"
namespace sa {
	class Rect {
	private:
		glm::vec2 m_position;
		glm::vec2 m_size;
	public:
		Rect();

		void setPosition(glm::vec2 position);
		glm::vec2 getPosition();

		void setSize(glm::vec2 size);
		glm::vec2 getSize();
	};
}
