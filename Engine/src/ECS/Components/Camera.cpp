#include "pch.h"
#include "Camera.h"

#include "Application.h"
#include "Graphics\RenderTarget.h"


namespace comp {

	void Camera::serialize(sa::Serializer& s) {

	}

	void Camera::deserialize(void* pDoc) {
		using namespace simdjson::ondemand;
		object& obj = *(object*)pDoc;
	}

	void Camera::onConstruct(sa::Entity* entity) {
		camera.setViewport({ { 0, 0 }, sa::Application::get()->getWindow()->getCurrentExtent() });
		if (!entity->hasComponent<comp::Transform>()) {
			entity->addComponent<comp::Transform>();
		}
	}

	void Camera::onDestroy(sa::Entity* entity) {

	}

	sa::RenderTarget* Camera::getRenderTarget() const {
		return m_pRenderTarget;
	}

	void Camera::setRenderTarget(sa::RenderTarget* pRenderTarget) {
		m_pRenderTarget = pRenderTarget;
	}

	sa::SceneCollection& Camera::getSceneCollection() {
		return m_sceneCollection;
	}

}