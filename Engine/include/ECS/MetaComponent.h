#pragma once
#include <entt\entt.hpp>

namespace sa {

	class MetaComponent {
	private:
		entt::meta_any m_handle;
		std::string m_typeName;

		friend class ComponentType;
		MetaComponent(entt::meta_any handle = {}, const std::string& typeName = "");
	public:

		template<typename T>
		T* cast();
		template<typename... Args>
		void emplace(Args&& ... args);
		
		void* data();

		const std::string& getTypeName() const;

		bool isValid() const;

		template<typename ...Args>
		entt::meta_any invoke(const std::string& name, Args&& ...args);

	};

	template<typename T>
	T* MetaComponent::cast() {
		if (!isValid())
			return nullptr;
		T** ptr = m_handle.try_cast<T*>();
		if (!ptr)
			return nullptr;
		return *ptr;
	}

	template<typename ...Args>
	inline void MetaComponent::emplace(Args && ...args) {
		m_handle.emplace(args...);
	}


	template<typename ...Args>
	inline entt::meta_any MetaComponent::invoke(const std::string& name, Args&& ...args) {
		return m_handle.invoke(entt::hashed_string(name.c_str()), args...);
	}

}