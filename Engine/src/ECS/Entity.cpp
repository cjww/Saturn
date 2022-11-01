#include "pch.h"
#include "Entity.h"

#include "Components.h"

#include "Scene.h"

#include "simdjson.h"

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


        type["parent"] = sol::property(
            [](const Entity& self) -> sol::lua_value {
                const Entity& parent = self.getParent();
                if (parent.isNull()) {
                    return sol::nil;
                }
                return parent;
            },
            [](Entity& self, const sol::lua_value& parent) {
                if (parent.is<sol::nil_t>()) {
                    self.orphan();
                    return;
                }
                self.setParent(parent.as<Entity>());
            }

        );
        
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


    void Entity::serialize(Serializer& s) {
        s.beginObject();
        s.value("id", (uint32_t)m_entity);

        Entity parent = getParent();
        if(!parent.isNull())
            s.value("parent", (uint32_t)parent);
        
        s.beginArray("components");
        auto& types = ComponentType::getRegisteredComponents();
        for (auto& type : types) {
            if (!hasComponent(type))
                continue;
            
            MetaComponent mt = getComponent(type);
            if (!mt.isValid())
                continue;

            ComponentBase** comp = (ComponentBase**)mt.data();

            s.beginObject();
            s.value("type", mt.getTypeName().c_str());
            (*comp)->serialize(s);
            s.endObject();
        }
        s.endArray();

        s.beginArray("scripts");

        auto scripts = m_pScene->getAssignedScripts(*this);
        for (auto& script : scripts) {
            script.serialize(s);
        }

        s.endArray();

        s.endObject();
    }

    void Entity::deserialize(void* pDoc) {
        using namespace simdjson::ondemand;
        object& obj = *(object*)pDoc;
        auto parent = obj["parent"];
        if (!parent.error()) {
            Entity parentEntity(m_pScene, (entt::entity)parent.get_uint64().value());
            setParent(parentEntity);
        }

        for (simdjson::ondemand::object compObj : obj["components"]) {

            std::string compName(compObj["type"].get_string().value());
            MetaComponent mt = addComponent(compName);
            ComponentBase** comp = (ComponentBase**)mt.data();
            (*comp)->deserialize(&compObj);
        }

        for (object script : obj["scripts"]) {
            addScript(script["path"].get_string().value());
        }
    }


    MetaComponent Entity::getComponent(ComponentType type) const {
        return type.invoke("get", *this);
    }
   
    MetaComponent Entity::getComponent(const std::string& name) const {
        ComponentType type = getComponentType(name);
        return getComponent(type);
    }

    bool Entity::hasComponent(ComponentType type) const {
        return *(bool*)type.invoke("has", *this).data();
    }

    bool Entity::hasComponent(const std::string& name) const {
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

    void Entity::setParent(const Entity& parent) {
        if (parent == *this || m_pScene != parent.m_pScene)
            return;
        m_pScene->getHierarchy().setParent(*this, parent);
        comp::Transform* transform = getComponent<comp::Transform>();
        comp::Transform* parentTransform = parent.getComponent<comp::Transform>();

        if (!transform || !parentTransform)
            return;
        transform->hasParent = true;
        transform->relativePosition = transform->position - parentTransform->position;
    }

    void Entity::orphan() const {
        m_pScene->getHierarchy().orphan(*this);
        comp::Transform* transform = getComponent<comp::Transform>();
        if (!transform)
            return;
        transform->hasParent = false;
    }

    const Entity& Entity::getParent() const {
        return m_pScene->getHierarchy().getParent(*this);
    }

    bool Entity::hasParent() const {
        return m_pScene->getHierarchy().hasParent(*this);
    }
    
    bool Entity::hasChildren() const {
        return m_pScene->getHierarchy().hasChildren(*this);
    }


    void Entity::destroy() {
        m_pScene->destroyEntity(*this);
    }

    bool Entity::isNull() const {
        return m_pScene == nullptr
            || !m_pScene->valid(m_entity);
    }

}