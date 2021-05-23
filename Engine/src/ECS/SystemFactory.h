#pragma once

#include "System.h"

class SystemFactory {
private:
	// Maps System to its name
	std::unordered_map<const char*, std::shared_ptr<System>> m_systems;
	// Maps each Systems signature to its name
	std::unordered_map<const char*, ComponentMask> m_signatures;
public:
	// creates System and register it in above maps
	template<typename T, typename ...Args>
	T* registerSystem(ComponentMask signature, Args&... args);

	// When a Component gets added or removed from an Entity all systems need to be notified
	void onEntitySignatureChange(Entity entity);

	// When an Entity is destroyed all systems need to be notified
	void onEntityDestroyed(Entity entity);
};

template<typename T, typename ...Args>
inline T* SystemFactory::registerSystem(ComponentMask signature, Args & ...args) {
	const char* name = typeid(T).name();

	if (m_systems.find(name) != m_systems.end()) {
		throw std::runtime_error("System already registered");
	}
	m_signatures[name] = signature;
	m_systems[name] = std::make_shared<T>(args...);
	return std::static_pointer_cast<T>(m_systems[name]).get();
}
