#pragma once
#include "ECS/Entity.h"

// Editor events
namespace sa {
	namespace editor_event {

		struct EntitySelected {
			Entity entity;
		};

		struct EntityDeselected {
			Entity entity;
		};
	};
}