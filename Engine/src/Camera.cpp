#include "pch.h"
#include "ECS/Components/Camera.h"

#include "Graphics/RenderTarget.h"
#include "ECS/Entity.h"
#include "ECS/Components/Transform.h"


namespace comp {
	Camera::Camera()
		: m_sceneCollection(sa::SceneCollection::CollectionMode::CONTINUOUS)
	{

	}

	void Camera::serialize(sa::Serializer& s) {

	}

	void Camera::deserialize(void* pDoc) {
		using namespace simdjson::ondemand;
		object& obj = *(object*)pDoc;
	}

	void Camera::onConstruct(sa::Entity* entity) {
		if (!entity->hasComponents<comp::Transform>()) {
			entity->addComponent<comp::Transform>();
		}
	}

	void Camera::onDestroy(sa::Entity* entity) {

	}

	const sa::AssetHolder<sa::RenderTarget>& Camera::getRenderTarget() const {
		return m_renderTarget;
	}

	void Camera::setRenderTarget(sa::RenderTarget* pRenderTarget) {
		m_renderTarget = pRenderTarget;
	}

	sa::SceneCollection& Camera::getSceneCollection() {
		return m_sceneCollection;
	}

}