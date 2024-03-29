#pragma once
#include <stdint.h>
#include "Format.hpp"
#include "Resources/ResourceManager.hpp"

namespace sa {

	typedef uint32_t ShaderStageFlags;
	enum ShaderStageFlagBits : ShaderStageFlags {
		VERTEX = 1,
		TESSELLATION_CONTROL = 2,
		TESSELLATION_EVALUATION = 4,
		GEOMETRY = 8,
		FRAGMENT = 16,
		COMPUTE = 32,
		ALL_GRAPHICS = VERTEX | TESSELLATION_CONTROL | TESSELLATION_EVALUATION | GEOMETRY | FRAGMENT,
		RAYGEN = 0x00000100,
		ANY_HIT = 0x00000200,
		CLOSEST_HIT = 0x00000400,
		MISS = 0x00000800,
		INTERSECTION = 0x00001000,
		CALLABLE = 0x00002000,
		TASK = 0x00000040,
		MESH = 0x00000080,
		SUBPASS_SHADING_HUAWEI = 0x00004000,
		CLUSTER_CULLING_HUAWEI = 0x00080000,
	};

	std::string to_string(sa::ShaderStageFlagBits stage);

	struct ShaderStageInfo {
		const char* pName;
		const uint32_t* pCode;
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