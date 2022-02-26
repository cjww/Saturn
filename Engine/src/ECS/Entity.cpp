#include "pch.h"
#include "Entity.h"

namespace sa {

    Entity::Entity(entt::registry* pRegistry, entt::entity entity)
        : m_pRegistry(pRegistry)
        , m_entity(entity)
    {

    }

    Entity::Entity()
        : m_pRegistry(nullptr)
        , m_entity(entt::null) {
    }

    MetaComponent Entity::getComponent(ComponentType type) const {
        return type.invoke("get", *this);
    }
   
    MetaComponent Entity::getComponent(const std::string& name) const {
        ComponentType type = getComponentType(name);
        return getComponent(type);
    }

    bool Entity::hasComponent(ComponentType type) const {
        return type.invoke("has", *this).cast<bool>();
    }

    bool Entity::hasComponent(const std::string name) const {
        ComponentType type = getComponentType(name);
        return hasComponent(type);
    }

    MetaComponent Entity::addComponent(ComponentType type) {
        return type.invoke("add", *this);
    }

    MetaComponent Entity::addComponent(const std::string& name) {
        ComponentType type = getComponentType(name);
        return addComponent(type);
    }

    void Entity::removeComponent(ComponentType type) {
        type.invoke("remove", *this);
    }

    void Entity::removeComponent(const std::string& name) {
        ComponentType type = getComponentType(name);
        removeComponent(type);
    }

    bool Entity::isNull() const {
        return m_pRegistry == nullptr
            || !m_pRegistry->valid(m_entity);
    }

}