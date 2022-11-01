#pragma once
#include "MetaComponent.h"

#include "Tools\utils.h"

namespace sa{

	class ComponentType {
	private:
		inline static std::vector<ComponentType> s_registeredComponents;

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

		bool operator==(const ComponentType& other);
		bool operator!=(const ComponentType& other);
		operator entt::meta_type() {
			return m_type;
		}

		template<typename T>
		static void registerComponent();

		static std::vector<ComponentType>& getRegisteredComponents();


	};

	
	ComponentType getComponentType(const std::string& name);
	
	template<typename T>
	std::string getComponentName();

	template<typename T>
	ComponentType getComponentType();

	template<typename T>
	inline std::string getComponentName() {
		std::string name = typeid(T).name();
		utils::stripTypeName(name);
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
		return { func.invoke({}, args...), m_name };
	}

	template<typename T>
	inline void ComponentType::registerComponent() {
		s_registeredComponents.push_back(getComponentType<T>());
	}

}