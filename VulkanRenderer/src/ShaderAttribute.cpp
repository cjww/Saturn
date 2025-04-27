#include "ShaderAttribute.h"
namespace sa {
    const ShaderAttribute* ShaderAttribute::getAttribute(const std::string& name) const {
        for (const ShaderAttribute& member : members) {
            std::string subName = name;
            if (const size_t offset = name.find_first_of('.'); offset != std::string::npos) {
                subName = name.substr(offset + 1);
                if (const ShaderAttribute* attribute = member.getAttribute(subName)) {
                    return attribute;
                }
            }
            else if (member.name == name) {
                return &member;
            }
        }
        return nullptr;
    }

    const ShaderAttribute* ShaderAttribute::getAttribute(const std::string& name, ShaderStageFlags stageFlags, ShaderAttributeTypeFlags typeFlags) const {
        std::vector<const ShaderAttribute*> attributes;
        findAttributes(stageFlags, typeFlags, attributes);
        for (const ShaderAttribute* attribute : attributes) {
            if (attribute->name == name) {
                return attribute;
            }
        }
        return nullptr;
    }

    void ShaderAttribute::findAttributes(ShaderStageFlags stageFlags, ShaderAttributeTypeFlags typeFlags,
        std::vector<const ShaderAttribute*> &attributes) const
    {
        for (const ShaderAttribute& member : members) {
            if (member.stage & stageFlags && member.type & typeFlags) {
                attributes.emplace_back(&member);
                member.findAttributes(stageFlags, typeFlags, attributes);
            }
        }
    }

    bool ShaderAttribute::hasAttribute(const std::string &name, ShaderStageFlags stageFlags, ShaderAttributeTypeFlags typeFlags) const {
        return getAttribute(name, stageFlags, typeFlags) != nullptr;
    }
}
