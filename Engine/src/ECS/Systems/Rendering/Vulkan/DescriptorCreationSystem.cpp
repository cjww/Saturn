#include "DescriptorCreationSystem.h"

namespace vk {
    bool DescriptorCreationSystem::removeEntity(EntityID entity) {
        if (System::removeEntity(entity)) {
            m_pDescManager->removeEntityDescriptorset(entity);
            return true;
        }
        return false;

    }

    bool DescriptorCreationSystem::addEntity(EntityID entity) {
        if (System::addEntity(entity)) {
            m_pDescManager->addEntityDescriptorSet(entity);
            return true;
        }
        return false;
    }

    DescriptorCreationSystem::DescriptorCreationSystem(DescriptorManager* pDescManager) 
        : m_pDescManager(pDescManager)
    {
    }

    void DescriptorCreationSystem::update(float dt) {
        
    }
}