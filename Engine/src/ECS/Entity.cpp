#include "Saturn/ECS/Entity.h"


Entity::Entity(ECSCoordinator* pCoordinator)
    : m_pCoordinator(pCoordinator)
    , m_id(m_pCoordinator->createEntity())
{

}

Entity::~Entity() {
    m_pCoordinator->destroyEntity(m_id);
}

EntityID Entity::getID() const  {
    return m_id;
}
