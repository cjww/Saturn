#pragma once
#include "Tools/utils.hpp"

namespace sa {
	struct Extent {
		uint32_t width, height;
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
		bool operator==(const Color& other) {
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
}