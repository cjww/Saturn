#pragma once

#include "entt\meta\meta.hpp"

namespace sa {

	class MetaComponent {
	private:
		entt::meta_any m_data;
	public:
		MetaComponent(entt::meta_any any = {});

		template<typename T>
		T* cast();

		bool isValid();
	};

	template<typename T>
	T* MetaComponent::cast() {
		if (!isValid())
			return nullptr;
		T** ptr = m_data.try_cast<T*>();
		if (!ptr)
			return nullptr;
		return *ptr;
	}

}