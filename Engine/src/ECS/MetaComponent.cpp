#include "MetaComponent.h"
namespace sa {
	
	MetaComponent::MetaComponent(entt::meta_any any)
		: m_data(any)
	{
	}
	bool MetaComponent::isValid() {
		return (bool)m_data;
	}
}