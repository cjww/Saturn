#pragma once

#include <ECS/System.h>
#include <ECS/ECSCoordinator.h>
#include <Renderer.hpp>
#include <ECS/Components.h>

namespace vk {

	class MeshRenderSystem : public System {
	private:
		ECSCoordinator* m_pCoordinator;
	public:
		MeshRenderSystem();

		virtual void update(float dt) override {}
		void draw() const;
	};

};