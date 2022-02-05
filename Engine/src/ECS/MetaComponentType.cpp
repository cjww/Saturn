#include "pch.h"
#include "MetaComponentType.h"

namespace sa {

	MetaComponentType::MetaComponentType(entt::meta_type type)
		: m_type(type)
	{
		if (m_type) {
			m_name = type.info().name();
			stripTypeName(m_name);
		}
	}

	bool MetaComponentType::isValid() const {
		return (bool)m_type;
	}

	std::string MetaComponentType::getName() const {
		return m_name;
	}

	uint32_t MetaComponentType::getTypeId() const {
		return m_type.info().hash();
	}

	void stripTypeName(std::string& str) {
		auto p = str.find_last_of("::");
		if (p != std::string::npos) {
			str = str.substr(p + 1);
		}
		str = str.substr(0, str.find_first_of("> "));
	}

	MetaComponentType getComponentType(const std::string& name) {
		return { entt::resolve(entt::hashed_string(name.c_str())) };
	}
}