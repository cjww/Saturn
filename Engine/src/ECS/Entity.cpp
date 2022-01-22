#include "Entity.h"

Entity::Entity(entt::registry* pRegistry, entt::entity entity)
    : m_pRegistry(pRegistry)
    , m_entity(entity)
{

}


Entity::~Entity() {
    
}

entt::entity Entity::getID() const  {
    return m_entity;
}
