#pragma once
#include "ECS/System.h"
#include "ECS/ECSCoordinator.h"
#include "ECS/Components.h"
#include "ECS\Systems\Rendering\Vulkan\DescriptorManager.h"
#include <unordered_map>

#include "structs.h"

namespace vk {
	class DescriptorCreationSystem : public System {
	private:

		DescriptorManager* m_pDescManager;

		friend class SystemFactory;
		virtual bool removeEntity(EntityID entity) override;
		virtual bool addEntity(EntityID entity) override;
	public:
		DescriptorCreationSystem(DescriptorManager* pDescManager);
		virtual void update(float dt);
	};
}