#pragma once

namespace sa {
	struct Extent {
		uint32_t width, height;
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