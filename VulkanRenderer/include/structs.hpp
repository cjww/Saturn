#pragma once
#include <stdint.h>
#include "Tools/utils.hpp"

#define SA_COLOR_WHITE sa::Color{ 1, 1, 1, 1 }
#define SA_COLOR_BLACK sa::Color{ 0, 0, 0, 1 }
#define SA_COLOR_RED sa::Color{ 1, 0, 0, 1 }
#define SA_COLOR_GREEN sa::Color{ 0, 1, 0, 1 }
#define SA_COLOR_BLUE sa::Color{ 0, 0, 1, 1 }

#define SA_COLOR_YELLOW sa::Color{ 1, 1, 0, 1 }
#define SA_COLOR_CYAN sa::Color{ 0, 1, 1, 1 }
#define SA_COLOR_MAGENTA sa::Color{ 1, 0, 1, 1 }


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
		float r, g, b, a;
		bool operator==(const Color& other) const {
			return utils::equals(r, other.r) &&
				utils::equals(g, other.g) &&
				utils::equals(b, other.b) &&
				utils::equals(a, other.a);
		}
	};
	
	
	struct Rect {
		Offset offset;
		Extent extent;
	};

	struct Bounds {
		float left, right, top, bottom;
	};
}