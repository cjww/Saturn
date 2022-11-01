#include "pch.h"
#include "Components.h"


namespace sa {

	void registerAllComponents() {
		registerComponentType<comp::Name>();
		registerComponentType<comp::Transform>();
		registerComponentType<comp::Light>();
		registerComponentType<comp::Model>();
	}

}