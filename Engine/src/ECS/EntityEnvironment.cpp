#include "pch.h"
#include "EntityEnvironment.h"

sa::details::EntityEnvironment::EntityEnvironment(const sol::state& state, sol::reference& fallback) {
	m_env = sol::environment(state, sol::create, fallback);
}
