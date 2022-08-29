#include "pch.h"
#include "Tools/utils.hpp"

namespace sa {
	namespace utils {
		bool equals(float a, float b) {
			return std::abs(a - b) < EPSILON;
		}
	}
}
