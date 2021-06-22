#include "MeshRenderSystem.h"
namespace vk {

	MeshRenderSystem::MeshRenderSystem(DescriptorManager* pDescManager)
		: m_pDescManager(pDescManager)
	{
		m_pCoordinator = ECSCoordinator::get();
	}

	void MeshRenderSystem::draw(uint32_t pipeline) const {
		vr::Renderer* renderer = vr::Renderer::get();

		for (const auto& entity : m_entitySet) {
			Model* modelComp = m_pCoordinator->getComponent<Model>(entity);
			ModelData* model = ResourceManager::get()->getModel(modelComp->modelID);

			Transform* transform = m_pCoordinator->getComponent<Transform>(entity);
			PerObjectBuffer perObject = {};
			perObject.worldMatrix = glm::mat4(1);
			perObject.worldMatrix = glm::translate(perObject.worldMatrix, transform->position);
			perObject.worldMatrix = glm::rotate(perObject.worldMatrix, transform->rotation.x, glm::vec3(1, 0, 0));
			perObject.worldMatrix = glm::rotate(perObject.worldMatrix, transform->rotation.y, glm::vec3(0, 1, 0));
			perObject.worldMatrix = glm::rotate(perObject.worldMatrix, transform->rotation.z, glm::vec3(0, 0, 1));
			perObject.worldMatrix = glm::scale(perObject.worldMatrix, transform->scale);

			memcpy(m_pDescManager->getBuffer(entity)->mappedData, &perObject, sizeof(perObject));
			renderer->updateDescriptorSet(m_pDescManager->getDescriptorSet(entity), 0, m_pDescManager->getBuffer(entity), nullptr, nullptr, false);
			renderer->bindDescriptorSet(m_pDescManager->getDescriptorSet(entity), pipeline);
			
			renderer->bindVertexBuffer(model->meshes[0].vertexBuffer);
			renderer->bindIndexBuffer(model->meshes[0].indexBuffer);
			renderer->drawIndexed(6, 1);

		}
	}
}
