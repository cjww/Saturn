#include "pch.h"
#include "Components.h"

#include "Entity.h"

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
	}

}