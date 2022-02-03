#pragma once
#include "MetaComponent.h"

namespace sa{

	class ComponentType {
	private:
		std::string m_name;
		entt::meta_type m_type;
	public:
		ComponentType() = default;
		ComponentType(entt::meta_type type);

		bool isValid() const;

		std::string getName() const;
		uint32_t getTypeId() const;


		template<typename ...Args>
		MetaComponent invoke(const std::string& name, Args&&... args);

	};

	
	void stripTypeName(std::string& str);
	ComponentType getComponentType(const std::string& name);
	
	template<typename T>
	std::string getComponentName();

	template<typename T>
	ComponentType getComponentType();

	template<typename T>
	inline std::string getComponentName() {
		std::string name = typeid(T).name();
		stripTypeName(name);
		return name;
	}

	template<typename T>
	inline ComponentType getComponentType() {
		return { entt::resolve(entt::hashed_string(getComponentName<T>().c_str())) };
	}

	template<typename ...Args>
	inline MetaComponent ComponentType::invoke(const std::string& name, Args && ...args) {
		auto func = m_type.func(entt::hashed_string(name.c_str()));
		if (!func)
			return {};
		return { func.invoke({}, args...) };
	}

}