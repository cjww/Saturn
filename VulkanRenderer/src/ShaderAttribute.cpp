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
}
