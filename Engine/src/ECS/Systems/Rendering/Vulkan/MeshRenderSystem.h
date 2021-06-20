#pragma once

#include "ECS/System.h"
#include "ECS/ECSCoordinator.h"
#include <Renderer.hpp>
#include "ECS/Components.h"
#include "ECS\Systems\Rendering\Vulkan\DescriptorManager.h"

namespace vk {

	class MeshRenderSystem : public System {
	private:
		ECSCoordinator* m_pCoordinator;
		DescriptorManager* m_pDescManager;
	public:
		MeshRenderSystem(DescriptorManager* pDescManager);

		virtual void update(float dt) override {}
		void draw(uint32_t pipeline) const;
	};

};