#pragma once
#include "Component.h"
namespace sa{

	class MetaComponentType {
	private:
		std::string m_name;
		entt::meta_type m_type;
	public:
		MetaComponentType() = default;
		MetaComponentType(entt::meta_type type);

		bool isValid() const;

		std::string getName() const;
		uint32_t getTypeId() const;


		template<typename ...Args>
		Component invoke(const std::string& name, Args&&... args);

		template<typename Comp>
		static void registerComponentType();

	};

	
	void stripTypeName(std::string& str);
	MetaComponentType getComponentType(const std::string& name);
	
	template<typename T>
	std::string getComponentName();

	template<typename T>
	MetaComponentType getComponentType();


	template<typename T>
	inline std::string getComponentName() {
		std::string name = typeid(T).name();
		stripTypeName(name);
		return name;
	}

	template<typename T>
	inline MetaComponentType getComponentType() {
		return { entt::resolve(entt::hashed_string(getComponentName<T>().c_str())) };
	}

	template<typename ...Args>
	inline Component MetaComponentType::invoke(const std::string& name, Args && ...args) {
		auto func = m_type.func(entt::hashed_string(name.c_str()));
		if (!func)
			return {};
		return { func.invoke({}, std::forward(args)...) };
	}

	template<typename Comp>
	inline void MetaComponentType::registerComponentType() {
		using namespace entt::literals;
		if constexpr (std::is_class_v<Comp>) {
			entt::meta<Comp>()
				.type(entt::hashed_string(getComponentName<Comp>().c_str()))
				.func<&Entity::hasComponent<Comp>>("has"_hs)
				.func<&Entity::getComponent<Comp>, entt::as_ref_t>("get"_hs)
				.func<&Entity::addComponent<Comp>, entt::as_ref_t>("add"_hs)
				.func<&Entity::removeComponent<Comp>>("remove"_hs)
				;
		}
	}

}