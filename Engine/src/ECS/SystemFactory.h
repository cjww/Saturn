#pragma once
#include <memory>

#include "System.h"
#include "ComponentQuery.h"

class SystemFactory {
private:
	// Maps System to its name
	std::unordered_map<const char*, std::unique_ptr<System>> m_systems;
	// Maps each Systems signature to its name
	std::unordered_map<const char*, ComponentQuery> m_queries;
public:
	// creates System and register it in above maps
	template<typename T, typename ...Args>
	T* registerSystem(ComponentQuery query, Args&... args);

	//Retrive pointer to system 
	template<typename T>
	T* getSystem() const;

	
	// When a Component gets added or removed from an Entity all systems need to be notified
	void onEntitySignatureAdd(EntityID entity, ComponentMask oldSignature, ComponentMask newSignature);
	void onEntitySignatureRemove(EntityID entity, ComponentMask oldSignature, ComponentMask newSignature);

	// When an Entity is destroyed all systems need to be notified
	void onEntityDestroyed(EntityID entity, ComponentMask signature);
};

template<typename T, typename ...Args>
inline T* SystemFactory::registerSystem(ComponentQuery query, Args& ...args) {
	const char* name = typeid(T).name();

	if (m_systems.find(name) != m_systems.end()) {
		throw std::runtime_error("System already registered");
	}
	m_queries.insert(std::make_pair(name, query));
	m_systems.insert(std::make_pair(name, std::make_unique<T>(args...)));
	return static_cast<T*>(m_systems.at(name).get());
}

template<typename T>
inline T* SystemFactory::getSystem() const {
	const char* name = typeid(T).name();
	if (m_systems.find(name) == m_systems.end()) {
		throw std::runtime_error("System not registered");
	}
	return m_systems.at(name);
}
