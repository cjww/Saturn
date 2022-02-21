#pragma once
#include "Tools\Vector.h"

namespace sa {
	class Rect {
	private:
		Vector2 m_position;
		Vector2 m_size;
	public:
		Rect();
		Rect(Vector2 position, Vector2 size);
		Rect(float x, float y, float width, float height);


		void setPosition(Vector2 position);
		Vector2 getPosition();

		void setSize(Vector2 size);
		Vector2 getSize();
	};
}
