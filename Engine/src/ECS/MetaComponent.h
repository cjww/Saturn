#pragma once
#include <entt\entt.hpp>

namespace sa {

	class MetaComponent {
	private:
		entt::meta_any m_data;
		std::string m_typeName;

		friend class ComponentType;
		MetaComponent(entt::meta_any any = {}, const std::string& typeName = "");
	public:

		template<typename T>
		T* cast();
		
		void* data();

		const std::string& getTypeName() const;

		bool isValid() const;
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