#pragma once
#include "ShaderInfoStructs.h"
#include "DescriptorSetStructs.h"

namespace sa {
    typedef uint32_t ShaderAttributeTypeFlags;
    enum ShaderAttributeTypeFlagBits : ShaderAttributeTypeFlags {
        SHADER_ATTRIBUTE_TYPE_STAGE_INPUT = 1,
        SHADER_ATTRIBUTE_TYPE_STAGE_OUTPUT = 2,
        SHADER_ATTRIBUTE_TYPE_DESCRIPTOR = 4,
        SHADER_ATTRIBUTE_TYPE_PUSH_CONSTANT = 8,
        SHADER_ATTRIBUTE_TYPE_ALL = ~0u,
    };

    struct ShaderAttribute {
        ShaderAttributeTypeFlagBits type;
        DescriptorType descriptorType;
        ShaderAttributeBaseType baseType;
        std::vector<uint32_t> arraySize;
        uint32_t vecSize;
        uint32_t columns;
        size_t size;
        uint32_t offset;
        std::string name;
        ShaderStageFlagBits stage;
        union {
            struct {
                uint32_t set;
                uint32_t binding;
            };
            uint32_t location;
        };

        std::vector<ShaderAttribute> members;

        const ShaderAttribute* getAttribute(const std::string& name) const;
        const ShaderAttribute* getAttribute(const std::string& name, ShaderStageFlags stageFlags, ShaderAttributeTypeFlags typeFlags = ShaderAttributeTypeFlagBits::SHADER_ATTRIBUTE_TYPE_ALL) const;
        void findAttributes(ShaderStageFlags stageFlags, ShaderAttributeTypeFlags typeFlags, std::vector<const ShaderAttribute*>& attributes) const;
        bool hasAttribute(const std::string& name, ShaderStageFlags stageFlags, ShaderAttributeTypeFlags typeFlags = ShaderAttributeTypeFlagBits::SHADER_ATTRIBUTE_TYPE_ALL) const;
    };
}