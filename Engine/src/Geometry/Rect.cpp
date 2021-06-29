#include "Rect.h"

namespace sa {

	Rect::Rect()
		: m_position(0, 0)
		, m_size(1, 1)
	{

	}

	void Rect::setPosition(glm::vec2 position) {
		m_position = position;
	}

	glm::vec2 Rect::getPosition() {
		return m_position;
	}

	void Rect::setSize(glm::vec2 size) {
		m_size = size;
	}

	glm::vec2 Rect::getSize() {
		return m_size;
	}

}