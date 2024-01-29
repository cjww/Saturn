#pragma once
#include "ECS/Components/Name.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/Light.h"
#include "ECS/Components/Model.h"
#include "ECS/Components/RigidBody.h"
#include "ECS/Components/BoxCollider.h"
#include "ECS/Components/SphereCollider.h"
#include "ECS/Components/Camera.h"

namespace sa {
	void registerAllComponents();

	template<typename Comp, std::enable_if_t<std::is_base_of_v<sa::ComponentBase, std::decay_t<Comp>>, bool> = true>
	void registerComponentType();

	


	template<typename Comp, std::enable_if_t<std::is_base_of_v<sa::ComponentBase, std::decay_t<Comp>>, bool>>
	inline void registerComponentType() {
		static bool registered = false;
		if (registered)
			return;

		registered = true;

		Entity::registerMetaFunctions<Comp>();
		ComponentType::registerComponent<Comp>();

		registerComponentLua<Comp>();
	}
}


