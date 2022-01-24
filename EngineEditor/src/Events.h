#pragma once
#include "ECS/Entity.h"

// Editor events
namespace event {

	struct EntitySelected {
		sa::Entity entity;
	};

	struct EntityDeselected {
		sa::Entity entity;
	};
};