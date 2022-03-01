#include "pch.h"
#include "Entity.h"

#include "Components.h"

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

    void updateEntityType() {
        auto type = LuaAccessable::luaReg<Entity>();
        for (const auto& name : LuaAccessable::getRegisteredComponents()) {
            auto varName = utils::toLower(name);
            type[varName] = sol::property([=](const Entity& self) -> sol::lua_value {
                ComponentType componentType = getComponentType(name);
                auto metaComp = componentType.invoke("get", self);
                return LuaAccessable::cast(metaComp);
            });
        }
        type["id"] = sol::readonly_property(&Entity::operator entt::id_type);
        type["name"] = sol::property(
            [](const Entity& e) -> std::string { return e.getComponent<comp::Name>()->name; },
            [](const Entity& e, const std::string& str) { e.getComponent<comp::Name>()->name = str; }
            );

    }

}