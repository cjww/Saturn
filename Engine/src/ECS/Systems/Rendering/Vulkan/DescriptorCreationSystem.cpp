#include "DescriptorCreationSystem.h"

namespace vk {
    bool DescriptorCreationSystem::removeEntity(EntityID entity) {
        return System::removeEntity(entity);
    }

    bool DescriptorCreationSystem::addEntity(EntityID entity) {
        if (System::addEntity(entity)) {
            m_uninitializedEntities.push(entity);
            return true;
        }
        return false;
    }

    DescriptorCreationSystem::DescriptorCreationSystem(vr::ShaderSet* pShaderSet) 
        : m_pShaderSet(pShaderSet)
    {
    }

    void DescriptorCreationSystem::update(float dt) {
        ECSCoordinator* c = ECSCoordinator::get();

        while (!m_uninitializedEntities.empty()) {
            EntityID id = m_uninitializedEntities.front();
            m_uninitializedEntities.pop();
        
            Transform* transform = c->getComponent<Transform>(id);

            vr::DescriptorSetPtr descSet = m_pShaderSet->getDescriptorSet(PER_OBJECT_SET);
            PerObjectBuffer bufferStruct = {};

            glm::mat4 proj = glm::perspective(glm::radians(90.f), 1000.0f / 600.0f, 0.01f, 100.0f);
            glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

            bufferStruct.worldMatrix = glm::mat4(1);
            //bufferStruct.worldMatrix = glm::translate(bufferStruct.worldMatrix, transform->position);
            //bufferStruct.worldMatrix = glm::scale(bufferStruct.worldMatrix, transform->scale);
            //glm::rotate(buffer.worldMatrix, transform->rotation);
            //bufferStruct.worldMatrix = proj * view * bufferStruct.worldMatrix;

            m_buffers[id][0] = vr::Renderer::get()->createUniformBuffer(sizeof(PerObjectBuffer), &bufferStruct);
            vr::Renderer::get()->updateDescriptorSet(descSet, 0, m_buffers[id][0], nullptr, nullptr, true);
            PerObjectBuffer* data = (PerObjectBuffer*)m_buffers[id][0]->mappedData;
            
            m_descriptorSets.insert(std::make_pair(id, descSet));

        }

    }

    vr::Buffer* DescriptorCreationSystem::getBuffer(EntityID entity, uint32_t binding) const {
        return m_buffers.at(entity).at(binding);
    }

    vr::DescriptorSetPtr DescriptorCreationSystem::getDescriptorSet(EntityID entity) const {
        return m_descriptorSets.at(entity);
    }

}