#include "Scene.h"

#include "ECS/Components.h"
#include "structs.h"
#include "Events.h"

namespace sa {

    void Scene::onModelConstruct(entt::registry& r, entt::entity entity) {
       comp::Model& m = r.get<comp::Model>(entity);
       sa::PerObjectBuffer b;
       b.worldMatrix = glm::mat4(1);
       m.buffer = vr::Renderer::get()->createUniformBuffer(sizeof(b), &b);
    }

    void Scene::onModelDestroy(entt::registry& r, entt::entity entity) {
        comp::Model& m = r.get<comp::Model>(entity);
        vr::Renderer::get()->destroyBuffer(m.buffer);
    }


    Scene::Scene() {
        m_registry.on_construct<comp::Model>().connect<&Scene::onModelConstruct>(this);
        m_registry.on_destroy<comp::Model>().connect<&Scene::onModelDestroy>(this);
    }

    entt::registry& Scene::getRegistry() {
        return m_registry;
    }

    Camera* Scene::createCamera(glm::vec2 extent) {
        m_cameras.push_back(std::make_unique<Camera>(extent));
        return m_cameras.back().get();
    }

    void Scene::addActiveCamera(Camera* camera) {
        m_activeCameras.insert(camera);
    }

    void Scene::removeActiveCamera(Camera* camera) {
        m_activeCameras.erase(camera);
    }

    const std::set<Camera*>& Scene::getActiveCameras() const {
        return m_activeCameras;
    }

    void Scene::update(float dt) {
        publish<event::OnUpdate>(dt);
    }

}