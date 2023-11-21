#include "pch.h"
#include "ECS/MetaComponent.h"

namespace sa {
	
	MetaComponent::MetaComponent(entt::meta_any handle, const std::string& typeName)
		: m_handle(handle)
		, m_typeName(typeName)
	{
		
	}

	void* MetaComponent::data() {
		return m_handle.data();
	}

	const std::string& MetaComponent::getTypeName() const {
		return m_typeName;
	}
	
	bool MetaComponent::isValid() const {
		return (bool)m_handle;
	}


}
