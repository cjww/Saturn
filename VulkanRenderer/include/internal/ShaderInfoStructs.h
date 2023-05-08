#pragma once
#include <stdint.h>
#include "Format.hpp"
#include "Resources/ResourceManager.hpp"

namespace sa {

	typedef uint32_t ShaderStageFlags;
	enum ShaderStageFlagBits : ShaderStageFlags {
		VERTEX = 1,
		TESSELATION_CONTROL = 2,
		TESSELATION_EVALUATION = 4,
		GEOMETRY = 8,
		FRAGMENT = 16,
		COMPUTE = 32,
	};


	struct ShaderStageInfo {
		char* pName;
		uint32_t* pCode;
		size_t codeLength;
		ShaderStageFlagBits stage;
	};

	struct PushConstantRange {
		ShaderStageFlags stageFlags;
		uint32_t offset;
		uint32_t size;
	};

	struct VertexInputAttributeDescription {
		uint32_t    location;
		uint32_t    binding;
		Format    format;
		uint32_t    offset;
	};

	struct VertexInputBindingDescription {
		uint32_t             binding;
		uint32_t             stride;
	};

	struct ShaderModuleInfo {
		ShaderStageFlagBits stage;
		ResourceID moduleID;
		std::string entryPointName;
	};
}