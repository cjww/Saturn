#include "pch.h"
#include "ECS/ComponentType.h"

namespace sa {

	ComponentType::ComponentType(entt::meta_type type)
		: m_type(type)
	{
		if (m_type) {
			m_name = type.info().name();
			utils::stripTypeName(m_name);
		}
	}

	bool ComponentType::isValid() const {
		return (bool)m_type;
	}

	std::string ComponentType::getName() const {
		return m_name;
	}

	uint32_t ComponentType::getTypeId() const {
		return m_type.info().hash();
	}

	bool ComponentType::operator==(const ComponentType& other) {
		return m_type == other.m_type;
	}

	bool ComponentType::operator!=(const ComponentType& other) {
		return m_type != other.m_type;
	}

	std::vector<ComponentType>& ComponentType::getRegisteredComponents() {
		return s_registeredComponents;
	}

	ComponentType getComponentType(const std::string& name) {
		return { entt::resolve(entt::hashed_string(name.c_str())) };
	}
}