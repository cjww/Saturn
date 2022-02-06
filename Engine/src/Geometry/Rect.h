#pragma once
#include "Tools\Vector.h"

namespace sa {
	class Rect {
	private:
		Vector2 m_position;
		Vector2 m_size;
	public:
		Rect();

		void setPosition(Vector2 position);
		Vector2 getPosition();

		void setSize(Vector2 size);
		Vector2 getSize();
	};
}
