#include "pch.h"
#include "Rect.h"

namespace sa {

	Rect::Rect()
		: m_position(0, 0)
		, m_size(1, 1)
	{

	}

	void Rect::setPosition(Vector2 position) {
		m_position = position;
	}

	Vector2 Rect::getPosition() {
		return m_position;
	}

	void Rect::setSize(Vector2 size) {
		m_size = size;
	}

	Vector2 Rect::getSize() {
		return m_size;
	}

}