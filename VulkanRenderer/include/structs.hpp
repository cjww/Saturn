#pragma once

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
	};
	
	
	struct Rect {
		Offset offset;
		Extent extent;
	};
}