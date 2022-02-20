#include "pch.h"
#include "MetaComponent.h"
namespace sa {
	
	MetaComponent::MetaComponent(entt::meta_any any, const std::string& typeName)
		: m_data(any), m_typeName(typeName)
	{
	}

	void* MetaComponent::data() {
		return m_data.data();
	}

	const std::string& MetaComponent::getTypeName() const {
		return m_typeName;
	}
	
	bool MetaComponent::isValid() const {
		return (bool)m_data;
	}
}