#include "pch.h"
#include "Entity.h"

#include "Components.h"
#include "Scene.h"

namespace sa {
    void Entity::reg() {
        auto type = getType();
     
        type["id"] = sol::readonly_property(&Entity::operator entt::id_type);
        type["name"] = sol::property(
            [](const Entity& e) -> std::string { return e.getComponent<comp::Name>()->name; },
            [](const Entity& e, const std::string& str) { e.getComponent<comp::Name>()->name = str; }
        );

        type["addScript"] = [](Entity& self, const std::string& path) {
            self.addScript(path);
        };
        type["removeScript"] = [](Entity& self, const std::string& name) {
            self.removeScript(name);
        };

        type["addComponent"] = [](Entity& self, const std::string& name) {
            self.addComponent(name);
        };
        type["removeComponent"] = [](Entity& self, const std::string& name) {
            self.removeComponent(name);
        };


        /*
        type["__newindex"] = [](Entity& self, const std::string& key, const sol::lua_value& value) {
            std::cout << "attempted to add new component " << key << " with value of type " <<
                sol::type_name(LuaAccessable::getState(), value.value().get_type())
                << std::endl;
        };
        */

       type["__index"] = [](const Entity& self, std::string key) -> sol::lua_value {
            key[0] = utils::toUpper(key[0]);
            std::optional<EntityScript> optScript = self.getScript(key);
            if (!optScript.has_value())
                return sol::nil;
            
            return optScript.value().env;
        };

    }

    sol::usertype<Entity>& Entity::getType() {
        static sol::usertype<Entity> type = LuaAccessable::registerType<Entity>();
        return type;
    }

    Entity::Entity(Scene* pScene, entt::entity entity)
        : m_pScene(pScene)
        , m_entity(entity)
        , m_pRegistry(pScene)
    {
    }

    Entity::Entity()
        : m_pScene(nullptr)
        , m_entity(entt::null)
        , m_pRegistry(nullptr)
    {
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

    void Entity::addScript(const std::filesystem::path& path) {
        m_pScene->addScript(*this, path);
    }

    void Entity::removeScript(const std::string& name) {
        m_pScene->removeScript(*this, name);
    }

    std::optional<EntityScript> Entity::getScript(const std::string& name) const {
        return m_pScene->getScript(*this, name);
    }

    bool Entity::isNull() const {
        return m_pScene == nullptr
            || !m_pScene->valid(m_entity);
    }

}