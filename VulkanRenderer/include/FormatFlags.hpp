#pragma once
#include <stdint.h>

namespace sa {

	typedef uint16_t FormatPrecisionFlags;
	typedef uint16_t FormatDimensionFlags;
	typedef uint16_t FormatTypeFlags;


	enum FormatPrecisionFlagBits : FormatPrecisionFlags {
		e64Bit = 1 << 0,
		e32Bit = 1 << 1,
		e16Bit = 1 << 2,
		e8Bit = 1 << 3,
		ANY_PRECISION = e64Bit | e32Bit | e16Bit | e8Bit
	};

	enum FormatDimensionFlagBits : FormatDimensionFlags {
		e1 = 1 << 4,
		e2 = 1 << 5,
		e3 = 1 << 6,
		e4 = 1 << 7,
		ANY_DIMENSION = e1 | e2 | e3 | e4
	};

	enum FormatTypeFlagBits : FormatTypeFlags {
		SFLOAT = 1 << 8,
		SINT = 1 << 9,
		UINT = 1 << 10,
		UNORM = 1 << 11,
		SRGB = 1 << 12,
		ANY_TYPE = SFLOAT | SINT | UINT | UNORM | SRGB
	};
}