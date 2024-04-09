#pragma once
#include <stdint.h>
#include "Tools/utils.hpp"

namespace sa {
	struct Extent {
		uint32_t width, height;
		bool operator==(const Extent& other) {
			return width == other.width &&
				height == other.height;
		}
		bool operator!=(const Extent& other) {
			return !(*this == other);
		}
	};

	struct Extent3D : public Extent {
		uint32_t depth;
	};

	struct Offset {
		int32_t x, y;
	};

	struct Point {
		double x, y;
	};


	struct Color {
		Color(float r, float g, float b, float a) {
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
		}
		float r, g, b, a;
		bool operator==(const Color& other) const {
			return utils::equals(r, other.r) &&
				utils::equals(g, other.g) &&
				utils::equals(b, other.b) &&
				utils::equals(a, other.a);
		}

		static Color White;
		static Color Black;
		static Color Red;
		static Color Green;
		static Color Blue;
		static Color Yellow;
		static Color Cyan;
		static Color Magenta;
	};
	inline Color Color::White = Color(1, 1, 1, 1);
	inline Color Color::Black = Color(0, 0, 0, 1);
	inline Color Color::Red = Color(1, 0, 0, 1);
	inline Color Color::Green = Color(0, 1, 0, 1);
	inline Color Color::Blue = Color(0, 0, 1, 1);
	inline Color Color::Yellow = Color(1, 1, 0, 1);
	inline Color Color::Cyan = Color(0, 1, 1, 1);
	inline Color Color::Magenta = Color(1, 0, 1, 1);
	
	
	struct Rect {
		Offset offset;
		Extent extent;
	};

	struct Bounds {
		float left, right, top, bottom;
	};
}