#include "MeshRenderSystem.h"
namespace vk {

	MeshRenderSystem::MeshRenderSystem() {
		m_pCoordinator = ECSCoordinator::get();
	}

	void MeshRenderSystem::draw() const {
		vr::Renderer* renderer = vr::Renderer::get();

		for (const auto& entity : m_entitySet) {
			Model* modelComp = m_pCoordinator->getComponent<Model>(entity);
			ModelData* model = ResourceManager::get()->getModel(modelComp->modelID);
			
			//Transform* transform = m_pCoordinator->getComponent<Transform>(entity);
			//renderer->updateDescriptorSet()


			renderer->bindVertexBuffer(model->meshes[0].vertexBuffer);
			renderer->bindIndexBuffer(model->meshes[0].indexBuffer);
			renderer->drawIndexed(6, 1);

		}
	}
}
