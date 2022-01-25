#include "pch.h"
#include "ComponentType.h"

namespace sa {

	ComponentType::ComponentType(entt::meta_type type)
		: m_type(type)
	{
		m_name = type.info().name();
		stripTypeName(m_name);
	}

	bool ComponentType::isValid() const {
		return (bool)m_type;
	}

	std::string ComponentType::getName() const {
		return m_name;
	}

	void stripTypeName(std::string& str) {
		auto p = str.find_last_of("::");
		if (p != std::string::npos) {
			str = str.substr(p + 1);
		}
		str = str.substr(0, str.find_first_of("> "));
	}

	ComponentType getComponentType(const std::string& name) {
		return { entt::resolve(entt::hashed_string(name.c_str())) };
	}
}