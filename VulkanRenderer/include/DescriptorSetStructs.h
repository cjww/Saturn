#pragma once
#include <vector>
#include "ShaderInfoStructs.h"

namespace vk {
	class Sampler;
}

namespace sa {

	enum class DescriptorType {
		SAMPLER,
		COMBINED_IMAGE_SAMPLER,
		SAMPLED_IMAGE,
		STORAGE_IMAGE,
		UNIFORM_TEXEL_BUFFER,
		STORAGE_TEXEL_BUFFER,
		UNIFORM_BUFFER,
		STORAGE_BUFFER,
		UNIFORM_BUFFER_DYNAMIC,
		STORAGE_BUFFER_DYNAMIC,
		INPUT_ATTACHMENT,
		INLINE_UNIFORM_BLOCK = 1000138000,
		ACCELERATION_STRUCTURE_KHR = 1000150000,
		ACCELERATION_STRUCTURE_NV = 1000165000,
		MUTABLE_VALVE = 1000351000,
		
		// Not actually a descriptor type
		PUSH_CONSTANT = -1
	};


	struct DescriptorSetLayoutBinding {
		ShaderStageFlags stageFlags;
		uint32_t binding;
		uint32_t descriptorCount;
		DescriptorType type;
		vk::Sampler* pImmutableSamplers;
	};

	struct DescriptorSetLayoutInfo {
		std::vector<DescriptorSetLayoutBinding> bindings;
		std::vector<size_t> sizes;
	};

	enum class ShaderAttributeType {
		UNKNOWN,
		VOID,
		BOOLEAN,
		SBYTE,
		UBYTE,
		SHORT,
		USHORT,
		INT,
		UINT,
		INT64,
		UINT64,
		ATOMIC_COUNTER,
		HALF,
		FLOAT,
		DOUBLE,
		STRUCT,
		IMAGE,
		SAMPLED_IMAGE,
		SAMPLER,
		ACCELERATION_STRUCTURE,
		RAY_QUERY
	};

	struct ShaderAttribute {
		DescriptorType descriptorType;
		ShaderAttributeType type;
		std::vector<uint32_t> arraySize;
		uint32_t vecSize;
		uint32_t columns;
		size_t size;
		uint32_t offset;
		std::string name;

		uint32_t set;
		uint32_t binding;
	};

}