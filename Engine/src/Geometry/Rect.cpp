#include "pch.h"
#include "Rect.h"

namespace sa {

	Rect::Rect()
		: m_position(0, 0)
		, m_size(1, 1)
	{

	}

	Rect::Rect(Vector2 position, Vector2 size) {
		m_position = position;
		m_size = size;
	}

	Rect::Rect(float x, float y, float width, float height) {
		m_position = Vector2(x, y);
		m_size = Vector2(width, height);
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