#include "pch.h"
#include "ShaderInfoStructs.h"

namespace sa {
	std::string to_string(sa::ShaderStageFlagBits stage) {
		const static std::unordered_map<sa::ShaderStageFlagBits, std::string> map = {
			{ sa::ShaderStageFlagBits::VERTEX, "Vertex" },
			{ sa::ShaderStageFlagBits::FRAGMENT, "Fragment" },
			{ sa::ShaderStageFlagBits::GEOMETRY, "Geometry" },
			{ sa::ShaderStageFlagBits::TESSELLATION_CONTROL, "Tessellation Control" },
			{ sa::ShaderStageFlagBits::TESSELLATION_EVALUATION, "Tessellation Evaluation" },
			{ sa::ShaderStageFlagBits::COMPUTE, "Compute" }
		};
	
		return map.at(stage);
	}

}