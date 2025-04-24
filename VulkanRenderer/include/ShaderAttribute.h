#pragma once
#include "ShaderInfoStructs.h"
#include "DescriptorSetStructs.h"

namespace sa {
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

        std::vector<ShaderAttribute> members;

        const ShaderAttribute* getAttribute(const std::string& name) const;
    };
}