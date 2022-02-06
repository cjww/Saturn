#pragma once
#include "Tools\Vector.h"

namespace sa {
	class Rect {
	private:
		Vector2f m_position;
		Vector2f m_size;
	public:
		Rect();

		void setPosition(Vector2f position);
		Vector2f getPosition();

		void setSize(Vector2f size);
		Vector2f getSize();
	};
}
