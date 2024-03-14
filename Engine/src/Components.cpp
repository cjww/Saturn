#include "pch.h"
#include "ECS/Components.h"

#include <ECS/Entity.h>

#include "Lua\LuaTypes.h"

namespace sa {
	
	void registerAllComponents() {
		registerComponentType<comp::Name>();
		registerComponentType<comp::Transform>();
		registerComponentType<comp::Light>();
		registerComponentType<comp::Model>();
		registerComponentType<comp::RigidBody>();
		registerComponentType<comp::BoxCollider>();
		registerComponentType<comp::SphereCollider>();
		registerComponentType<comp::Camera>();
		registerComponentType<comp::ShadowEmitter>();
	}

}