#pragma once
#include "ECS/Entity.h"

// Editor events
namespace sa {
	namespace event {

		struct EntitySelected {
			Entity entity;
		};

		struct EntityDeselected {
			Entity entity;
		};
	};
}