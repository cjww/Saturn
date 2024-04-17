#include "pch.h"
#include "ECS/Components/Camera.h"

#include "Graphics/RenderTarget.h"
#include "ECS/Entity.h"
#include "ECS/Components/Transform.h"


namespace comp {
	void Camera::onRendertargetResize(const sa::engine_event::RenderTargetResized& e) {
		// reset aspect ratio if current rendertarget was resized
		if (m_renderTarget.isHolding() && e.renderTargetID == m_renderTarget.getID()) {
			camera.setAspectRatio(static_cast<float>(e.newExtent.width) / e.newExtent.height);
		}
	}

	Camera::Camera()
		: sceneCollection(sa::SceneCollection::CollectionMode::CONTINUOUS)
	{

	}

	Camera::Camera(const Camera& other) : Camera() {
		camera = other.camera;
		m_renderTarget = other.m_renderTarget;
	}

	Camera::Camera(Camera&& other) : Camera() {
		camera = other.camera;
		m_renderTarget = other.m_renderTarget;
	}

	Camera& Camera::operator=(const Camera& other) {
		camera = other.camera;
		m_renderTarget = other.m_renderTarget;
		return *this;
	}

	Camera& Camera::operator=(Camera&& other) {
		camera = other.camera;
		m_renderTarget = other.m_renderTarget;
		return *this;
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
		if(pRenderTarget)
			camera.setAspectRatio(static_cast<float>(pRenderTarget->getExtent().width) / pRenderTarget->getExtent().height);
	}


}