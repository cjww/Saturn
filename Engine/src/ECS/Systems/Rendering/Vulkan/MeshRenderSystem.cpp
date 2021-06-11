#include "MeshRenderSystem.h"
namespace vk {

	MeshRenderSystem::MeshRenderSystem(DescriptorCreationSystem* pDescSystem)
		: m_pDescSystem(pDescSystem)
	{
		m_pCoordinator = ECSCoordinator::get();
	}

	void MeshRenderSystem::draw(uint32_t pipeline) const {
		vr::Renderer* renderer = vr::Renderer::get();

		for (const auto& entity : m_entitySet) {
			Model* modelComp = m_pCoordinator->getComponent<Model>(entity);
			ModelData* model = ResourceManager::get()->getModel(modelComp->modelID);
			/*
			Transform* transform = m_pCoordinator->getComponent<Transform>(entity);
			*/
			//renderer->updateDescriptorSet(m_pDescSystem->getDescriptorSet(entity), 0, m_pDescSystem->getBuffer(entity, 0), nullptr, nullptr, false);
			renderer->bindDescriptorSet(m_pDescSystem->getDescriptorSet(entity), pipeline);
			
			renderer->bindVertexBuffer(model->meshes[0].vertexBuffer);
			renderer->bindIndexBuffer(model->meshes[0].indexBuffer);
			renderer->drawIndexed(6, 1);

		}
	}
}
