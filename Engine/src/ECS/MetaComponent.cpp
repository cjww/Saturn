#include "pch.h"
#include "MetaComponent.h"
namespace sa {
	
	MetaComponent::MetaComponent(entt::meta_any any)
		: m_data(any)
	{
	}

	void* MetaComponent::data() {
		return m_data.data();
	}
	
	bool MetaComponent::isValid() {
		return (bool)m_data;
	}
}